#include "TRRenderer.h"

#include "glm/gtc/matrix_transform.hpp"

#include "TRShaderPipeline.h"

#include <cmath>

namespace TinyRenderer
{

	TRRenderer::TRRenderer(int width, int height)
		: m_backBuffer(nullptr), m_frontBuffer(nullptr)
	{
		//Double buffer to avoid flickering
		m_backBuffer = std::make_shared<TRFrameBuffer>(width, height);
		m_frontBuffer = std::make_shared<TRFrameBuffer>(width, height);

		//Setup viewport matrix (ndc space -> screen space)
		m_viewportMatrix = calcViewPortMatrix(width, height);
	}

	void TRRenderer::loadDrawableMesh(const std::string &filename)
	{
		TRDrawableMesh drawable;
		drawable.loadMeshFromFile(filename);
		m_drawableMeshes.push_back(drawable);
	}

	void TRRenderer::unloadDrawableMesh()
	{
		for (size_t i = 0; i < m_drawableMeshes.size(); ++i)
		{
			m_drawableMeshes[i].unload();
		}
		std::vector<TRDrawableMesh>().swap(m_drawableMeshes);
	}

	void TRRenderer::setViewMatrix(const glm::mat4 &view)
	{
		m_mvp_dirty = true;
		m_viewMatrix = view;
	}

	void TRRenderer::setModelMatrix(const glm::mat4 &model)
	{
		m_mvp_dirty = true;
		m_modelMatrix = model;
	}

	void TRRenderer::setProjectMatrix(const glm::mat4 &project, float near, float far)
	{
		m_mvp_dirty = true;
		m_projectMatrix = project;
		m_frustum_near_far = glm::vec2(near, far);
	}

	void TRRenderer::setShaderPipeline(TRShaderPipeline::ptr shader)
	{
		m_shader_handler = shader;
	}

	glm::mat4 TRRenderer::getMVPMatrix()
	{
		if (m_mvp_dirty)
		{
			m_mvp_matrix = m_projectMatrix * m_viewMatrix * m_modelMatrix;
			m_mvp_dirty = false;
		}
		return m_mvp_matrix;
	}

	void TRRenderer::clearColor(glm::vec4 color)
	{
		m_backBuffer->clear(color);
	}

	void TRRenderer::renderAllDrawableMeshes()
	{
		if (m_shader_handler == nullptr)
		{
			m_shader_handler = std::make_shared<TRDefaultShaderPipeline>();
		}
		
		//Load the matrices
		m_shader_handler->setModelMatrix(m_modelMatrix);
		m_shader_handler->setViewProjectMatrix(m_projectMatrix * m_viewMatrix);

		//Draw a mesh step by step
		m_clip_cull_profile.m_num_cliped_triangles = 0;
		m_clip_cull_profile.m_num_culled_triangles = 0;
		std::vector<TRShaderPipeline::VertexData> rasterized_points;
		rasterized_points.reserve(m_backBuffer->getWidth() * m_backBuffer->getHeight());
		for (size_t m = 0; m < m_drawableMeshes.size(); ++m)
		{
			const auto& vertices = m_drawableMeshes[m].getVerticesAttrib();
			const auto& faces = m_drawableMeshes[m].getMeshFaces();
			for (size_t f = 0; f < faces.size(); ++f)
			{
				//A triangle as primitive
				TRShaderPipeline::VertexData v[3];
				{
					v[0].pos = vertices.vpositions[faces[f].vposIndex[0]];
					v[0].col = glm::vec3(vertices.vcolors[faces[f].vposIndex[0]]);
					v[0].nor = vertices.vnormals[faces[f].vnorIndex[0]];
					v[0].tex = vertices.vtexcoords[faces[f].vtexIndex[0]];

					v[1].pos = vertices.vpositions[faces[f].vposIndex[1]];
					v[1].col = glm::vec3(vertices.vcolors[faces[f].vposIndex[1]]);
					v[1].nor = vertices.vnormals[faces[f].vnorIndex[1]];
					v[1].tex = vertices.vtexcoords[faces[f].vtexIndex[1]];

					v[2].pos = vertices.vpositions[faces[f].vposIndex[2]];
					v[2].col = glm::vec3(vertices.vcolors[faces[f].vposIndex[2]]);
					v[2].nor = vertices.vnormals[faces[f].vnorIndex[2]];
					v[2].tex = vertices.vtexcoords[faces[f].vtexIndex[2]];
				}

				//Vertex shader stage
				std::vector<TRShaderPipeline::VertexData> clipped_vertices;
				{
					//Vertex shader
					{
						m_shader_handler->vertexShader(v[0]);
						m_shader_handler->vertexShader(v[1]);
						m_shader_handler->vertexShader(v[2]);
					}

					//Homogeneous space cliping
					{

						clipped_vertices = cliping(v[0], v[1], v[2]);
						if (clipped_vertices.empty())
						{
							++m_clip_cull_profile.m_num_cliped_triangles;
							continue;
						}
					}

					//Perspective division
					for (auto &vert : clipped_vertices)
					{
						//Perspective correction before rasterization
						TRShaderPipeline::VertexData::prePerspCorrection(vert);

						//From clip space -> ndc space
						vert.cpos /= vert.cpos.w;
					}
				}

				int num_verts = clipped_vertices.size();
				for (int i = 0; i < num_verts - 2; ++i)
				{
					//Triangle assembly
					TRShaderPipeline::VertexData vert[3] = {
							clipped_vertices[0],
							clipped_vertices[i + 1],
							clipped_vertices[i + 2] };

					//Backface culling
					{
						if (isTowardBackFace(vert[0].cpos, vert[1].cpos, vert[2].cpos))
						{
							++m_clip_cull_profile.m_num_culled_triangles;
							continue;
						}
					}

					//Rasterization stage
					{
						//Transform to screen space & Rasterization
						{
							vert[0].spos = glm::ivec2(m_viewportMatrix * vert[0].cpos + glm::vec4(0.5f));
							vert[1].spos = glm::ivec2(m_viewportMatrix * vert[1].cpos + glm::vec4(0.5f));
							vert[2].spos = glm::ivec2(m_viewportMatrix * vert[2].cpos + glm::vec4(0.5f));


							// 2: change here
							//m_shader_handler->rasterize_wire(vert[0], vert[1], vert[2],
								//m_backBuffer->getWidth(), m_backBuffer->getHeight(), rasterized_points);
							m_shader_handler->rasterize_fill_edge_function(vert[0], vert[1], vert[2],
								m_backBuffer->getWidth(), m_backBuffer->getHeight(), rasterized_points);
						}
					}

					//Fragment shader & Depth testing
					{
						for (auto &points : rasterized_points)
						{
							// 3: Implement depth testing here
							// Note: You should use m_backBuffer->readDepth() and points.spos to read the depth in buffer
							//       points.cpos.z is the depth of current fragment
							{
								//Perspective correction after rasterization
								/*
								TRShaderPipeline::VertexData::aftPrespCorrection(points);
								glm::vec4 fragColor;
								m_shader_handler->fragmentShader(points, fragColor);
								m_backBuffer->writeColor(points.spos.x, points.spos.y, fragColor);
								m_backBuffer->writeDepth(points.spos.x, points.spos.y, points.cpos.z);
								*/
								float currentDepth = points.cpos.z;
								float storedDepth = m_backBuffer->readDepth(points.spos.x, points.spos.y);

								if (currentDepth < storedDepth) {
					
									TRShaderPipeline::VertexData::aftPrespCorrection(points);
									glm::vec4 fragColor;
									m_shader_handler->fragmentShader(points, fragColor);
									m_backBuffer->writeColor(points.spos.x, points.spos.y, fragColor);
									m_backBuffer->writeDepth(points.spos.x, points.spos.y, currentDepth);
								}
							}
						}
					}

					rasterized_points.clear();
				}
			}

		}

		//Swap double buffers
		{
			std::swap(m_backBuffer, m_frontBuffer);
		}
		
	}

	unsigned char* TRRenderer::commitRenderedColorBuffer()
	{
		return m_frontBuffer->getColorBuffer();
	}

	unsigned int TRRenderer::getNumberOfClipFaces() const
	{
		return m_clip_cull_profile.m_num_cliped_triangles;
	}

	unsigned int TRRenderer::getNumberOfCullFaces() const
	{
		return m_clip_cull_profile.m_num_culled_triangles;
	}

	std::vector<TRShaderPipeline::VertexData> TRRenderer::cliping(
		const TRShaderPipeline::VertexData &v0,
		const TRShaderPipeline::VertexData &v1,
		const TRShaderPipeline::VertexData &v2) const
	{
		//Clipping in the homogeneous clipping space

		if (v0.cpos.w < m_frustum_near_far.x && v1.cpos.w < m_frustum_near_far.x && v2.cpos.w < m_frustum_near_far.x)
			return {};
		if (v0.cpos.w > m_frustum_near_far.y && v1.cpos.w > m_frustum_near_far.y && v2.cpos.w > m_frustum_near_far.y)
			return {};
		if (v0.cpos.x > v0.cpos.w && v1.cpos.x > v1.cpos.w && v2.cpos.x > v2.cpos.w)
			return {};
		if (v0.cpos.x < -v0.cpos.w && v1.cpos.x < -v1.cpos.w && v2.cpos.x < -v2.cpos.w)
			return {};
		if (v0.cpos.y > v0.cpos.w && v1.cpos.y > v1.cpos.w && v2.cpos.y > v2.cpos.w)
			return {};
		if (v0.cpos.y < -v0.cpos.w && v1.cpos.y < -v1.cpos.w && v2.cpos.y < -v2.cpos.w)
			return {};
		if (v0.cpos.z > v0.cpos.w && v1.cpos.z > v1.cpos.w && v2.cpos.z > v2.cpos.w)
			return {};
		if (v0.cpos.z < -v0.cpos.w && v1.cpos.z < -v1.cpos.w && v2.cpos.z < -v2.cpos.w)
			return {};

		return { v0, v1, v2 };
	}

	bool TRRenderer::isTowardBackFace(const glm::vec4 &v0, const glm::vec4 &v1, const glm::vec4 &v2) const
	{
		//Back face culling in the ndc space

		
		glm::vec3 edge1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
		glm::vec3 edge2(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

		glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
		glm::vec3 view = glm::vec3(0, 0, 1);

		return glm::dot(normal, view) < 0;

	}

	glm::mat4 TRRenderer::calcViewPortMatrix(int width, int height)
	{
		//Setup viewport matrix (ndc space -> screen space)
		glm::mat4 vpMat;
		float hwidth = width * 0.5f;
		float hheight = height * 0.5f;
		vpMat[0][0] = hwidth; vpMat[0][1] = 0.0f;    vpMat[0][2] = 0.0f; vpMat[0][3] = 0.0f;
		vpMat[1][0] = 0.0f;	  vpMat[1][1] =-hheight; vpMat[1][2] = 0.0f; vpMat[1][3] = 0.0f;
		vpMat[2][0] = 0.0f;   vpMat[2][1] = 0.0f;    vpMat[2][2] = 1.0f; vpMat[2][3] = 0.0f;
		vpMat[3][0] = hwidth; vpMat[3][1] = hheight; vpMat[3][2] = 0.0f; vpMat[3][3] = 0.0f;
		return vpMat;
	}

	glm::mat4 TRRenderer::calcViewMatrix(glm::vec3 camera, glm::vec3 target, glm::vec3 worldUp)
	{
		//Setup view matrix (world space -> camera space)
		glm::mat4 vMat;
		glm::vec3 zAxis = glm::normalize(camera - target);
		glm::vec3 xAxis = glm::normalize(glm::cross(worldUp, zAxis));
		glm::vec3 yAxis = glm::normalize(glm::cross(zAxis, xAxis));

		vMat[0][0] = xAxis.x; vMat[0][1] = yAxis.x; vMat[0][2] = zAxis.x; vMat[0][3] = 0.0f;
		vMat[1][0] = xAxis.y; vMat[1][1] = yAxis.y; vMat[1][2] = zAxis.y; vMat[1][3] = 0.0f;
		vMat[2][0] = xAxis.z; vMat[2][1] = yAxis.z; vMat[2][2] = zAxis.z; vMat[2][3] = 0.0f;
		vMat[3][0] = -glm::dot(xAxis, camera);
		vMat[3][1] = -glm::dot(yAxis, camera);
		vMat[3][2] = -glm::dot(zAxis, camera);
		vMat[3][3] = 1.0f;

		return vMat;
	}

	glm::mat4 TRRenderer::calcPerspProjectMatrix(float fovy, float aspect, float near, float far)
	{
		//Setup perspective matrix (camera space -> homogeneous space)
		return glm::perspective(fovy, aspect, near, far);
		glm::mat4 pMat = glm::mat4(1.0f);

		float rFovy = fovy * 3.14159265358979323846 / 180;
		const float tanHalfFovy = std::tan(rFovy * 0.5f);
		float f_n = far - near;
		pMat[0][0] = 1.0f/(aspect*tanHalfFovy); pMat[0][1] = 0.0f;				pMat[0][2] = 0.0f;					pMat[0][3] = 0.0f;
		pMat[1][0] = 0.0f;						pMat[1][1] = 1.0f/tanHalfFovy;  pMat[1][2] = 0.0f;					pMat[1][3] = 0.0f;
		pMat[2][0] = 0.0f;						pMat[2][1] = 0.0f;			    pMat[2][2] = -(far+near)/ f_n;		pMat[2][3] = -1.0f;
		pMat[3][0] = 0.0f;						pMat[3][1] = 0.0f;				pMat[3][2] = -2.0f*near*far/ f_n;	pMat[3][3] = 0.0f;

		return pMat;
	}

	glm::mat4 TRRenderer::calcOrthoProjectMatrix(float left, float right, float bottom, float top, float near, float far)
	{
		//Setup orthogonal matrix (camera space -> homogeneous space)
		glm::mat4 pMat;
		pMat[0][0] = 2.0f/(right - left); pMat[0][1] = 0.0f;              pMat[0][2] = 0.0f;                   pMat[0][3] = 0.0f;
		pMat[1][0] = 0.0f;				  pMat[1][1] = 2.0f/(top-bottom); pMat[1][2] = 0.0f;                   pMat[1][3] = 0.0f;
		pMat[2][0] = 0.0f;                pMat[2][1] = 0.0f;              pMat[2][2] = -2.0f/(far-near);       pMat[2][3] = 0.0f;
		pMat[3][0] = 0.0f;                pMat[3][1] = 0.0f;              pMat[3][2] = -(far+near)/(far-near); pMat[3][3] = 1.0f;
		return pMat;
	}

}
