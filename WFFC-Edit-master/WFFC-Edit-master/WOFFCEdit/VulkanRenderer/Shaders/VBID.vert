#version 450
#extension GL_ARB_shader_draw_parameters : enable

layout (location = 0) in vec4 inPos;
layout (location = 1) in vec4 inColor;

layout (binding = 0) uniform UBO
{
	mat4 projection;
	mat4 model;
	mat4 view;
}ubo;

layout (location = 0) out flat uint outDrawID;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	uint drawID = gl_DrawIDARB;
	gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPos.xyz, 1.0);
	outDrawID = drawID;
}