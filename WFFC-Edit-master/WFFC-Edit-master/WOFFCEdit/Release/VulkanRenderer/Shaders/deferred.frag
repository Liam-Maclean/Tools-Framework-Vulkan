#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

//structs
struct spotLight
{
	vec4 position;
	vec4 diffuse;
	vec3 specular;
	float cutOff;
};
struct pointLight
{
	vec4 position;
	vec4 diffuse;
	vec4 specular;
};
struct directionalLight
{
	vec4 direction;
	vec4 diffuse;
	vec4 specular;
};

//In
//attribute locations
layout (location = 0) in vec2 inUV;

//UBO's
layout (binding = 0) uniform sampler2D samplerPosition;
layout (binding = 1) uniform sampler2D samplerNormal;
layout (binding = 2) uniform sampler2D samplerAlbedo;
layout (std430, binding = 3) readonly buffer spotLightBuffer
{
	spotLight spotLightData[];
};
layout (std430, binding = 4) readonly buffer pointLightBuffer
{
	pointLight pointLightData[];
};
layout (std430, binding = 5) readonly buffer directionalLightBuffer
{
	directionalLight directionalLightData[];
};
layout (binding = 6) uniform LightingEnabled
{
	float isEnabled;
}lightingEnabled;

//Out
layout (location = 0) out vec4 outFragColor;

//Main section of the shader
void main() 
{
	//temporary variables
	vec4 ambientColor = vec4(0.1f, 0.1f, 0.1f, 1.0f);
	vec4 lightDir = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	float lightIntensity = 0.0f;
	vec4 diffuseComponent = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	
	// Get G-Buffer values
	vec4 fragPos = texture(samplerPosition, inUV);
	vec3 normal = texture(samplerNormal, inUV).rgb;
	vec4 albedo = texture(samplerAlbedo, inUV);
	
	//Set the lowest possible ambient color for the scene
	diffuseComponent = ambientColor;
	
	if (lightingEnabled.isEnabled == 1)
	{
		for(int i = 0; i < 1; i++)
		{
			//Test lighting for a single directional light structure
			lightDir = directionalLightData[i].direction;
			
			//get light intensity of the dot product of the normal and light direction
			lightIntensity = max(dot(normal, lightDir.xyz), 0.0f);
			
			//if the pixel is lit
			if (lightIntensity > 0.0f)
			{
				diffuseComponent += (directionalLightData[i].diffuse * lightIntensity);
				//diffuseComponent = normalize(diffuseComponent);
			}
		}
	
	
		outFragColor = (diffuseComponent * albedo);
	}
	if (lightingEnabled.isEnabled == 2)
	{
		outFragColor = vec4(normal.xyz, 1.0f);
	}
	if (lightingEnabled.isEnabled == 0)
	{
		outFragColor = albedo;
	}
		

}