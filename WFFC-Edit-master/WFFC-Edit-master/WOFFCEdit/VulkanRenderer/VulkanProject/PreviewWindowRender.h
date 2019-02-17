#pragma once
#include "Window.h"
#include "Renderer.h"
#include "glm/glm.hpp"
#include "Camera.h"
#include "Plane.h"
#include "PlaneMesh.h"
#include "ImportedModel.h"
#include <atltypes.h>
class PreviewWindowRender :
	public VulkanWindow
{
public:
	PreviewWindowRender(Renderer* renderer, int width, int height)
	: VulkanWindow(renderer, width, height)
	{
		InitialiseVulkanApplication();
	}

	PreviewWindowRender(Renderer* renderer, int width, int height, HINSTANCE instance, HWND window)
		:VulkanWindow(renderer, width, height, instance, window)
	{

	}

	~PreviewWindowRender();

	inline void ToggleLighting() {
		if (lightingToggled != 1)
		{
			lightingToggled = 1;
		}
		else
		{
			lightingToggled = 0;
		}
	};

	struct uboVS {
		glm::mat4 projection;
		glm::mat4 view;
	};
	struct vertice {
		VkPipelineVertexInputStateCreateInfo inputState;
		VkVertexInputBindingDescription bindingDescriptions;
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions;
	};

	struct UboDataDynamic
	{
		glm::mat4 *model = nullptr;
	} uboDataDynamic;

	void _CreateGeometry();
	void CreateCamera();
	void InitialiseVulkanApplication();
	void Update();
	void DrawFrame() override;
	void _CreateGraphicsPipeline() override;
	void UpdateUniformBuffer(uint32_t currentImage);
	VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage);
	void _CreateRenderPass() override;
	void _CreateCommandBuffers() override;
	void _CreateVertexDescriptions();
	void _CreateDescriptorPool() override;
	void _CreateDescriptorSets() override;
	void _CreateDescriptorSetLayout() override;
	void _CreateShaderBuffer(VkDevice device, VkDeviceSize size, VkBuffer* vertexBuffer, VkDeviceMemory* vertexBufferMemory, VkBufferUsageFlagBits bufferStage, void *data);
	void SetUpUniformBuffers();
	void CreateGBuffer();
	void CreateDeferredCommandBuffers();

	ImportedModel* houseModel;

	Camera* camera;

	vertice vertices;
	uboVS offScreenUniformVSData;
	uboVS fullScreenUniformVSData;
	ScreenTarget* screenTarget;
	VkDescriptorSet descriptorSet;
	VkDescriptorSet descriptorPool;

	vk::wrappers::GFrameBuffer deferredOffScreenFrameBuffer;
	VkSampler colorSampler;


	size_t dynamicAlignment;
	size_t dynamicTextureAlignment;

	VkDescriptorSetLayout offScreenDescriptorSetLayout;
	VkDescriptorSetLayout deferredDescriptorSetLayout;

	VkCommandBuffer offScreenCmdBuffer;
	VkSemaphore presentCompleteSemaphore;
	VkSemaphore renderCompleteSemaphore;
	VkSemaphore offScreenSemaphore;


	vk::wrappers::Buffer offScreenVertexUBOBuffer;
	vk::wrappers::Buffer lightingEnabledBuffer;
	vk::wrappers::Buffer dynamicUboBuffer;

private:
	bool renderChange = false;
	float lightingToggled = 0;

};

