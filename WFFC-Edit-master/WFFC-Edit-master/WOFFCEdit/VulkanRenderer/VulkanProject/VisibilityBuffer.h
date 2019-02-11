#pragma once
#include "Window.h"
#include "Shared.h"
#include "BaseModel.h"
#include "ImportedModel.h"
#include "PlaneMesh.h"
#include "Camera.h"
class VisibilityBuffer :
	public VulkanWindow
{
public:

	

	VisibilityBuffer(Renderer* renderer, int width, int height)
		:VulkanWindow(renderer, width, height)
	{
		InitialiseVulkanApplication();
	}
	~VisibilityBuffer();
	struct uboVS {
		glm::mat4 projection;
		glm::mat4 model;
		glm::mat4 view;
	};
	struct vertice {
		VkPipelineVertexInputStateCreateInfo inputState;
		VkVertexInputBindingDescription bindingDescriptions;
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions;
	};
	struct {
		vk::wrappers::ModelBuffers floor;
		vk::wrappers::ModelBuffers quad;
		vk::wrappers::ModelBuffers cube;
	}models;
	struct {
		VkDescriptorSet cube;
		VkDescriptorSet house;
	}descriptorSets;

	struct
	{
		VkDescriptorSetLayout IDLayout;
		VkDescriptorSetLayout triangleFilterLayout;
		VkDescriptorSetLayout compositionLayout;
	}descriptorSetLayouts;


	void InitialiseVulkanApplication();
	void CreateCamera();
	void _GenerateQuads();
	void CreateVBuffer();
	void _CreateVertexBuffer(VkDevice device, VkDeviceSize size, VkBuffer * vertexBuffer, VkDeviceMemory * vertexBufferMemory, void * data);
	void _CreateIndexBuffer(VkDevice device, VkDeviceSize size, VkBuffer * indexBuffer, VkDeviceMemory * indexBufferMemory, void * data);
	void _CreateUniformBuffer(VkDevice device, uboVS uboVertexData, VkBuffer * vertexBuffer, VkDeviceMemory * vertexBufferMemory);
	void _SetUpUniformBuffers();
	void _CreateVIDCommandBuffers();
	void _CreateVertexFilteringCommandBuffers();
	void _CreateCommandBuffers() override;
	void _CreateVertexDescriptions();
	void _CreateDescriptorPool() override;
	void _CreateDescriptorSets() override;
	void _CreateDescriptorSetLayout() override;
	void Update() override;
	void DrawFrame() override;
	void _LoadAssets();
	void _CreateGraphicsPipeline() override;

	PlaneMesh* houseModel;
	ImportedModel* houseModel2;
	vertice vertices;
	uboVS IDMatricesVSData;
	vk::wrappers::Buffer IDMatricesUBOBuffer;
	vk::wrappers::VFrameBuffer IDFrameBuffer;
	VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage);
	VkSampler colorSampler;
	VkCommandBuffer IDCmdBuffer;
	VkSemaphore IDPassSemaphore;

	VkDescriptorSet IDDescriptorSet;
	VkDescriptorSet compositionDescriptorSet;

	Camera* camera;

	VkSemaphore presentCompleteSemaphore;
	VkSemaphore renderCompleteSemaphore;
	VkSemaphore offScreenSemaphore;

};

