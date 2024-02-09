#version 330 core 
layout (location = 0) in float Type;
layout (location = 1) in vec3 Position;
layout (location = 2) in vec3 Velocity;
layout (location = 3) in float Age;
layout (location = 4) in float size;
out float Type0;
out vec3 Position0;
out vec3 Velocity0;
out float Age0;
out float Size0;

void main()
{
	Type0 = Type;
	Position0 = Position;
	Velocity0 = Velocity;
	Age0 = Age;
    Size0 = size;
}
