#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstddef>
#include "../shader_helper.h"
#include "../drawable.h"
#include "stb_image.h"

class Cube : public Drawable{
public:
   void init(GLuint pid){
      _pid = pid;
      if(_pid == 0) exit(-1);

      glUseProgram(_pid);

      glGenVertexArrays(1, &_vao);
      glBindVertexArray(_vao);

      GLfloat indexed_position[] = {-1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         -1.0f,  1.0f,  1.0f,
         -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         -1.0f,  1.0f, -1.0f};

      GLuint index[] = {0, 1, 2,//front
         0, 2, 3,
         1, 5, 6,//right
         1, 6, 2,
         5, 4, 7,//back
         5, 7, 6,
         4, 0, 3,//left
         4, 3, 7,
         3, 2, 6,//top
         3, 6, 7,
         1, 0, 4,//bottom
         1, 4, 5};

      GLfloat position[36*3];
      GLfloat texture_coord[36 * 2];  // 纹理坐标

      //we need to generate positions one by one in order to have per surface vertex
      generate_positions(indexed_position, index, position, texture_coord);

      GLfloat per_surface_normals[] = {0, 0, 1, 0, 0, 1, 0, 0, 1,
         0, 0, 1, 0, 0, 1, 0, 0, 1,
         1, 0, 0, 1, 0, 0, 1, 0, 0,
         1, 0, 0, 1, 0, 0, 1, 0, 0,
         0, 0, -1, 0, 0, -1, 0, 0, -1,
         0, 0, -1, 0, 0, -1, 0, 0, -1,
         -1, 0, 0, -1, 0, 0, -1, 0, 0,
         -1, 0, 0, -1, 0, 0, -1, 0, 0,
         0, 1, 0, 0, 1, 0, 0, 1, 0,
         0, 1, 0, 0, 1, 0, 0, 1, 0,
         0, -1, 0, 0, -1, 0, 0, -1, 0,
         0, -1, 0, 0, -1, 0, 0, -1, 0
      };


      glGenBuffers(1, &_vbo_pos);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_pos);
      glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);

      GLuint id_pos = glGetAttribLocation(_pid, "position");
      glEnableVertexAttribArray(id_pos);
      glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      _num_vertices = sizeof(position)/sizeof(GLfloat);
      _num_indices = sizeof(index)/sizeof(GLuint);

      glGenBuffers(1, &_vbo_sur_norm);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_sur_norm);
      glBufferData(GL_ARRAY_BUFFER, sizeof(per_surface_normals), per_surface_normals, GL_STATIC_DRAW);

      id_pos = glGetAttribLocation(_pid, "surface_normal");
      glEnableVertexAttribArray(id_pos);
      glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      color[0] = 1.0;
      color[1] = 1.0;
      color[2] = 1.0;

      // 传入纹理坐标
      glGenBuffers(1, &_vbo_tex);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_tex);
      glBufferData(GL_ARRAY_BUFFER, sizeof(texture_coord), texture_coord, GL_STATIC_DRAW);
      GLuint vtexcoord_id = glGetAttribLocation(_pid, "uv");
      glEnableVertexAttribArray(vtexcoord_id);
      glVertexAttribPointer(vtexcoord_id, 2, GL_FLOAT, GL_FALSE, 0, 0);

      glBindVertexArray(0);

      // 地面纹理
      {
          unsigned char* texture_data;
          int width, height, nrChannels;
          texture_data = stbi_load("../resources/image/grass.jpg", &width, &height, &nrChannels, 0);
          std::cout << width << " " << height << " " << std::endl;
          glGenTextures(1, &_texture_ground);
          glBindTexture(GL_TEXTURE_2D, _texture_ground);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          if (texture_data)
          {
              std::cout << "load success" << std::endl;
              glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data);
              glGenerateMipmap(GL_TEXTURE_2D);
          }
          else
          {
              std::cout << "fail to load image" << std::endl;
          }
          stbi_image_free(texture_data);
          glBindTexture(GL_TEXTURE_2D, 0);
      }
      // 雪地纹理
      {
          unsigned char* texture_data;
          int width, height, nrChannels;
          texture_data = stbi_load("../resources/image/snow.jpg", &width, &height, &nrChannels, 0);
          std::cout << width << " " << height << " " << std::endl;
          glGenTextures(1, &_texture_snow);
          glBindTexture(GL_TEXTURE_2D, _texture_snow);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          if (texture_data)
          {
              std::cout << "load success" << std::endl;
              glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data);
              glGenerateMipmap(GL_TEXTURE_2D);
          }
          else
          {
              std::cout << "fail to load image" << std::endl;
          }
          stbi_image_free(texture_data);
          glBindTexture(GL_TEXTURE_2D, 0);
      }
   }

   void draw(glm::mat4x4 model, glm::mat4x4 view, glm::mat4x4 projection){
      glUseProgram(_pid);
      glBindVertexArray(_vao);

      // 绑定纹理
      glActiveTexture(GL_TEXTURE0);
      if (isWinter)
      {
          glBindTexture(GL_TEXTURE_2D, _texture_snow);
      }
      else
      {
          glBindTexture(GL_TEXTURE_2D, _texture_ground);
      }
      glUniform1i(glGetUniformLocation(_pid, "ground_texture"), 0);

      glUniformMatrix4fv( glGetUniformLocation(_pid, "model"), 1, GL_FALSE, glm::value_ptr(model));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "view"), 1, GL_FALSE, glm::value_ptr(view));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

      // glDrawElements(GL_TRIANGLES, _num_indices, GL_UNSIGNED_INT, 0);
      glDrawArrays(GL_TRIANGLES, 0, _num_vertices);

      glUseProgram(0);
      glBindVertexArray(0);
   }

   void set_color(float r, float g, float b){
      color[0] = r;
      color[1] = g;
      color[2] = b;
   }

   void draw(){
      glUseProgram(_pid);
      glBindVertexArray(_vao);

      // 绑定纹理
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, _texture_ground);
      glUniform1i(glGetUniformLocation(_pid, "ground_texture"), 0);

      glUniform3fv( glGetUniformLocation(_pid, "light_position"), 1, this->light_position);
      glUniform3fv( glGetUniformLocation(_pid, "camera_position"), 1, this->camera_position);

      glUniform3fv( glGetUniformLocation(_pid, "shape_color"), 1, this->color);
      glUniform4fv( glGetUniformLocation(_pid, "clip_coord"), 1, this->clip_coord);
      glUniform1ui( glGetUniformLocation(_pid, "shadow_mapping_effect"), this->shadow_mapping_effect);
      glUniform1ui( glGetUniformLocation(_pid, "shadow_buffer_tex_size"), this->shadow_buffer_texture_width); //width == height in this case
      glUniform1ui( glGetUniformLocation(_pid, "window_width"), this->window_width);
      glUniform1ui( glGetUniformLocation(_pid, "window_height"), this->window_height);

      glUniform3fv( glGetUniformLocation(_pid, "sun_dir"), 1, this->sun_dir);
      glUniform3fv( glGetUniformLocation(_pid, "sun_col"), 1, this->sun_col);

      if(has_shadow_buffer){
         glUniformMatrix4fv( glGetUniformLocation(_pid, "shadow_matrix"), 1, GL_FALSE, glm::value_ptr(this->shadow_matrix));

         glActiveTexture(GL_TEXTURE1);
         glBindTexture(GL_TEXTURE_2D, _shadow_texture_id);
         GLuint tex_id = glGetUniformLocation(_pid, "shadow_buffer_tex");
         glUniform1i(tex_id, 1 /*GL_TEXTURE0*/);
      }

      draw(this->model_matrix, this->view_matrix, this->projection_matrix);

      if(has_shadow_buffer){
         glBindTexture(GL_TEXTURE_2D, 0);
      }

      glUseProgram(0);
      glBindVertexArray(0);
   }

   void cleanup(){
      glDeleteBuffers(1, &_vbo_pos);
      glDeleteBuffers(1, &_vbo_sur_norm);
      glDeleteVertexArrays(1, &_vao);
      glDeleteProgram(_pid);
   }

protected:
   GLuint _texture_ground; // 地面贴图
   GLuint _texture_snow;
   GLuint _vbo_tex; // 贴图坐标
   GLuint _vao;
   GLuint _vbo_pos;
   GLuint _vbo_sur_norm;
   unsigned int _num_indices;
   unsigned int _num_vertices;

   float color[3];

   //generate real position from indexed ones (to do per surface normals)
   void generate_positions(GLfloat indexed_position[24], GLuint index[36], GLfloat position[36*3], GLfloat texture_coord[36 * 2]){  // 生成顶点和纹理坐标
       
       for (int i = 0; i < 36; i++) {
         position[i*3+0] = indexed_position[index[i]*3+0];
         position[i*3+1] = indexed_position[index[i]*3+1];
         position[i*3+2] = indexed_position[index[i]*3+2];
      }
      // 纹理坐标
      for (int i = 0; i < 6; i++)
      {
          texture_coord[i * 12 + 0] = 0.0f;
          texture_coord[i * 12 + 1] = 0.0f;

          texture_coord[i * 12 + 2] = 1.0f;
          texture_coord[i * 12 + 3] = 0.0f;

          texture_coord[i * 12 + 4] = 1.0f;
          texture_coord[i * 12 + 5] = 1.0f;

          texture_coord[i * 12 + 6] = 0.0f;
          texture_coord[i * 12 + 7] = 0.0f;

          texture_coord[i * 12 + 8] = 1.0f;
          texture_coord[i * 12 + 9] = 1.0f;

          texture_coord[i * 12 + 10] = 0.0f;
          texture_coord[i * 12 + 11] = 1.0f;

      }
   }
public:
    int isWinter = 1;
};
