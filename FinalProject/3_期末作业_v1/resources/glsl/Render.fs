#version 330 

in vec3 pos;
out vec4 color;
uniform sampler2D snowflower;

void main()
{
    vec4 texColor = texture(snowflower,gl_PointCoord);
	texColor.a = texColor.a;
    //if(texColor.r < 0.3)discard;
    //color = texColor;
    //color = vec4(1.0f,1.0f,1.0f,texColor.a);
	color = texColor;
}
