#version 330 core
layout (location = 0)in vec3 position;
layout (location = 1)in float size;

out vec3 pos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main()
{
	pos = position;
	gl_PointSize = size;
	gl_Position = projection * view * model * vec4(position,1.0f);
}
