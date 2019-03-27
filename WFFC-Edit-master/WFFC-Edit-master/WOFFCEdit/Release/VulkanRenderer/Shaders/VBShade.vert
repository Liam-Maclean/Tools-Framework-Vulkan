#version 450

//Pass to the fragment shader
layout (location = 0) out vec2 outUV;

void main()
{
	//Produces fullscreen triangle for fullscreen quad
	vec4 position;
	position.x = (gl_VertexIndex == 2) ? 3.0f : -1.0f;
	position.y = (gl_VertexIndex == 0) ? -3.0f : 1.0f;
	position.zw = vec2(0.0f, 1.0f);
	
	
	outUV = position.xy;
	//vec2 outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	//gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);
	gl_Position = position;
}

