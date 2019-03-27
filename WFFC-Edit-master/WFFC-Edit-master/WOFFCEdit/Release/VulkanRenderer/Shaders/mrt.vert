#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

//In
//Vertex Attributes
layout (location = 0) in vec4 inPos;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec4 inNormal;

//UBO's
//standard UBO
layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 view;
} ubo;

//Dynamic UBO for model matrix
layout (binding = 1) uniform UboModelMatrix {
	mat4 model; 
} uboModelMatrix;

//Out
layout (location = 0) out vec4 outWorldPos;
layout (location = 1) out vec4 outColor;
layout (location = 2) out vec2 outUV;
layout (location = 3) out vec4 outNormal;
out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	gl_Position = ubo.projection * ubo.view * uboModelMatrix.model * vec4(inPos.xyz, 1.0f);
	
	//outNormal = uboModelMatrix.model * vec4(inNormal.xyz, 1.0f);
	//outNormal = normalize(outNormal);
	
	outNormal = inNormal;
	outWorldPos = uboModelMatrix.model* vec4(inPos.xyz, 1.0f);
	outWorldPos.y = -outWorldPos.y;
	outColor = inColor;
	
	outUV = inColor.xy;
	outUV.t = 1.0 - outUV.t;
}
