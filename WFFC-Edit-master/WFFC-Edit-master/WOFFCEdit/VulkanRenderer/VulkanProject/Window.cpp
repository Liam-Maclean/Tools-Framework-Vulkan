#pragma once
#define NOMINMAX
#include "Window.h"
#include "Shared.h"
#include <iostream>
#include <cstdlib>
#include <assert.h>
#include <vector>
#include <sstream>
#include <set>
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"


//Constructor
VulkanWindow::VulkanWindow(Renderer* renderer, int width, int height)
{
	_renderer = renderer;

	_InitWindow(width, height);
}

VulkanWindow::VulkanWindow(Renderer * renderer, int width, int height, HINSTANCE hinstance, HWND hwnd)
{
	_renderer = renderer;
	_win32_instance = hinstance;
	_win32_window = hwnd;

	//_InitWindow(width, height);
}

//Prepares the base of the scene
void VulkanWindow::PrepareScene()
{
	VulkanWindow::_InitSurface();
	VulkanWindow::_CreateSemaphores();
	VulkanWindow::_CreateSwapChain();
	VulkanWindow::_CreateRenderPass();
	VulkanWindow::_CreateImageViews();
	VulkanWindow::_CreateCommandPool();
	VulkanWindow::_CreateDepthResources();
	VulkanWindow::_CreatePipelineCache();
	VulkanWindow::_CreateFramebuffers();
	VulkanWindow::_CreateCommandBuffers();
	VulkanWindow::_CreateUniformBuffer();
}

void VulkanWindow::_CreatePipelineCache()
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	vk::tools::ErrorCheck(vkCreatePipelineCache(_renderer->GetVulkanDevice(), &pipelineCacheCreateInfo, nullptr, &pipelineCache));
}

//Deconstructor
VulkanWindow::~VulkanWindow()
{
	vkQueueWaitIdle(_renderer->GetVulkanGraphicsQueue());

	_CleanUpSwapChain();

	vkDestroySampler(_renderer->GetVulkanDevice(), _textureSampler, nullptr);
	vkDestroyImageView(_renderer->GetVulkanDevice(), _textureImageView, nullptr);

	vkDestroyImage(_renderer->GetVulkanDevice(), _textureImage, nullptr);
	vkFreeMemory(_renderer->GetVulkanDevice(), _textureImageMemory, nullptr);

	vkDestroyDescriptorPool(_renderer->GetVulkanDevice(), _descriptorPool, nullptr);

	vkDestroyDescriptorSetLayout(_renderer->GetVulkanDevice(), _descriptorSetLayout, nullptr);
	
	for (size_t i = 0; i < _swapChainImages.size(); i++)
	{
		vkDestroyBuffer(_renderer->GetVulkanDevice(), _uniformBuffers[i], nullptr);
		vkFreeMemory(_renderer->GetVulkanDevice(), _uniformBuffersMemory[i], nullptr);
	}

	
	vkDestroyBuffer(_renderer->GetVulkanDevice(), _indexBuffer, nullptr);
	vkFreeMemory(_renderer->GetVulkanDevice(), _indexBufferMemory, nullptr);

	vkDestroyBuffer(_renderer->GetVulkanDevice(), _vertexBuffer, nullptr);
	vkFreeMemory(_renderer->GetVulkanDevice(), _vertexBufferMemory, nullptr);
	_DeInitSemaphores();
	_DeInitCommandPool();
	_DeInitSurface();
	_DeInitWindow();
}

//Method for debug recall, called on resize of screen in GLFW
static void _FrameBufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
	app->_frameBufferResized = true;
}

//Queries the swap chain support that the VKdevice can handle
SwapChainSupportDetails VulkanWindow::_QuerySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	//query the surface capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);


	//Query the surface formats from the device
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);

	//if there is more than one surface format
	if (formatCount != 0)
	{
		//resize and add the formats to the structure
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.formats.data());
	}


	//Query the presentation modes on the device
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

//Method to choose what swapchain surface format we're using
VkSurfaceFormatKHR VulkanWindow::_ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	//if the surface has no prefered format
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		//Use nonlinear standard
		return { VK_FORMAT_B8G8R8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	//if it has no prefered format loop through available formats
	for (const auto& availableFormat : availableFormats)
	{
		//if UNORM and NONLINEAR_KHR is available
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			//return that format
			return availableFormat;
		}
	}

	//if all else fails return the first format that exists
	return availableFormats[0];
}

//Method to choose what present mode for the swapchain we want to use
VkPresentModeKHR VulkanWindow::_ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
	//*VK_PRESENT_MODE_IMMEDIATE_KHR = Images submitted and transfered to screen straight away
	//*VK_PRESENT_MODE_FIFO_KHR = Swapchain queue, images at front of queue are taken when display is refreshed
	//*VK_PRESENT_MODE_FIFO_RELAXED_KHR = Like Previous but if application late and queue empty transfer right away
	//*VK_PRESENT_MODE_MAILBOX_KHR = like FIFO (normal) but used for triple buffering methods

	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	//loop through available present modes
	for (const auto& availablePresentMode : availablePresentModes)
	{
		//if mailbox is available
		if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR)
		{
			//return that available present mode
			return availablePresentMode;
		}
		//otherwise if available presentMode is immediate, change to that
		//if mailbox isn't available
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			bestMode = availablePresentMode;
		}
	}

	//return default FIFO if fail to find prefered type
	return bestMode;
}

//Method to choose what capabilities the swapchain will have (resolution, bitdepth, alpha blending)
VkExtent2D VulkanWindow::_ChooseSwapExtent(const VkSurfaceCapabilitiesKHR & capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		//glfwGetFramebufferSize(_window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(200),
			static_cast<uint32_t>(200)
		};

		//actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		//actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

//Method to find and filter the queue families of the device we have chosen
QueueFamilyIndices  VulkanWindow::_FindQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	//query device for family queue support properties
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	//loop through queue families to find specific queue types and flags
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		//query the physical device for surface support
		VkBool32 presentSupport = VK_NULL_HANDLE;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);

		//if suitable
		if (queueFamily.queueCount > 0 && presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			//break out of the loop and return indices
			break;
		}

		i++;
	}

	return indices;
}

//Method to create shader module for graphics pipeline using bytecode passed in (spv)
VkShaderModule VulkanWindow::_CreateShaderModule(const std::vector<char>& code)
{
	//set up module create info
	VkShaderModuleCreateInfo shader_module_create_info = {};
	shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_create_info.codeSize = code.size();
	shader_module_create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

	//create module create info
	VkShaderModule shaderModule;

	//run and error check result of vkCreateShaderModule
	vk::tools::ErrorCheck(vkCreateShaderModule(_renderer->GetVulkanDevice(), &shader_module_create_info, nullptr, &shaderModule));

	//return created shader module
	return shaderModule;
}

//Method for creating render passes (BASE: Forward rendering standard render attachments)
void VulkanWindow::_CreateRenderPass()
{

	VkAttachmentDescription color_attachment = {};
	color_attachment.format = _swapChainImageFormat;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depth_attachment = {};
	depth_attachment.format = _FindDepthFormat();
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = nullptr;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = nullptr;
	subpass.pResolveAttachments = nullptr;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask - VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	// Subpass dependencies for layout transitions
	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { color_attachment, depth_attachment };
	VkRenderPassCreateInfo render_pass_create_info = {};
	render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
	render_pass_create_info.pAttachments = attachments.data();
	render_pass_create_info.subpassCount = 1;
	render_pass_create_info.pSubpasses = &subpass;
	render_pass_create_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
	render_pass_create_info.pDependencies = dependencies.data();

	vk::tools::ErrorCheck(vkCreateRenderPass(_renderer->GetVulkanDevice(), &render_pass_create_info, nullptr, &_renderPass));
}

//Method to create graphics pipeline (BASE: Forward rendering standard and wireframe)
void VulkanWindow::_CreateGraphicsPipeline()
{

	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
	//loads the vert and frag shader from files using Utility read function
	//*(Shared.h)
	auto vertShaderCode = vk::tools::ReadShaderFile("C:/Users/Liam Maclean/Documents/Github/VulkanProject/VulkanProject/Shaders/vert.spv");
	auto fragShaderCode = vk::tools::ReadShaderFile("C:/Users/Liam Maclean/Documents/Github/VulkanProject/VulkanProject/Shaders/frag.spv");

	//Creates a shader module using the _CreateShaderModule method
	vertShaderModule = _CreateShaderModule(vertShaderCode);
	fragShaderModule = _CreateShaderModule(fragShaderCode);

	//set up create info handle for vert shader
	VkPipelineShaderStageCreateInfo vert_shader_create_info = {};
	vert_shader_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_shader_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_shader_create_info.module = vertShaderModule;
	vert_shader_create_info.pName = "main";

	//set up create info handle for frag shader
	VkPipelineShaderStageCreateInfo frag_shader_create_info = {};
	frag_shader_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_shader_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_shader_create_info.module = fragShaderModule;
	frag_shader_create_info.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vert_shader_create_info, frag_shader_create_info };

	//auto bindingDescription = Vertex::GetBindingDescription();
	//auto attributeDescriptions = Vertex::GetAttributeDescriptions();

	//Vertex input create info setup
	VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = 1;
	//vertex_input_info.pVertexBindingDescriptions = &bindingDescription;
//	vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	//vertex_input_info.pVertexAttributeDescriptions = attributeDescriptions.data();

	//input assembly create info setup
	VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = {};
	input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_create_info.primitiveRestartEnable = VK_FALSE;


	//viewport, describe reguin of the framebuffer output is rendered to
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)_swapChainExtent.width;
	viewport.height = (float)_swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	//Source rectangle (region of the image we are drawing to viewport)
	VkRect2D scissor = {};
	scissor.offset = { 0,0 };
	scissor.extent = _swapChainExtent;

	//viewport create info setup
	VkPipelineViewportStateCreateInfo viewport_state_create_info = {};
	viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_create_info.viewportCount = 1;
	viewport_state_create_info.pViewports = &viewport;
	viewport_state_create_info.scissorCount = 1;
	viewport_state_create_info.pScissors = &scissor;

	//Rasterizer stage create info setup
	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {};
	rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_state_create_info.depthClampEnable = VK_FALSE;
	rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE; //for skipping rasterization
	rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL; //FILL, LINE and POINT
	rasterization_state_create_info.lineWidth = 1.0f;
	rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterization_state_create_info.depthBiasEnable = VK_FALSE; //used for shadow mapping

	//Multisampling stage create info setup
	VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {};
	multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_state_create_info.sampleShadingEnable = VK_FALSE;
	multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	//Depth and Stencil testing
	VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {};
	depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_info.depthTestEnable = VK_TRUE;
	depth_stencil_info.depthWriteEnable = VK_TRUE;
	depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
	depth_stencil_info.stencilTestEnable = VK_FALSE;

	//ColorBlending stage (Can be used for alpha blending)
	VkPipelineColorBlendAttachmentState color_blend_attachment = {};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {};
	color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_state_create_info.logicOpEnable = VK_FALSE;
	color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
	color_blend_state_create_info.attachmentCount = 1;
	color_blend_state_create_info.pAttachments = &color_blend_attachment;
	color_blend_state_create_info.blendConstants[0] = 0.0f;
	color_blend_state_create_info.blendConstants[1] = 0.0f;
	color_blend_state_create_info.blendConstants[2] = 0.0f;
	color_blend_state_create_info.blendConstants[3] = 0.0f;

	VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {};
	dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state_create_info.dynamicStateCount = 2;
	dynamic_state_create_info.pDynamicStates = _dynamicStates;


	//Pipeline layout, used to create uniform values in shaders a bit like globals
	//that can be changed at runtime to alter behaviour of shaders rather than
	//having to recreate the shaders
	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
	pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_info.setLayoutCount = 1;
	pipeline_layout_create_info.pSetLayouts = &_descriptorSetLayout;

	//create and error check the pipeline layout
	vk::tools::ErrorCheck(vkCreatePipelineLayout(_renderer->GetVulkanDevice(), &pipeline_layout_create_info, nullptr, &_pipelineLayout[PipelineType::standard]));
	vk::tools::ErrorCheck(vkCreatePipelineLayout(_renderer->GetVulkanDevice(), &pipeline_layout_create_info, nullptr, &_pipelineLayout[PipelineType::wireframe]));

	VkGraphicsPipelineCreateInfo pipeline_create_info = {};
	pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_create_info.stageCount = 2;
	pipeline_create_info.pStages = shaderStages;
	pipeline_create_info.pVertexInputState = &vertex_input_info;
	pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
	pipeline_create_info.pViewportState = &viewport_state_create_info;
	pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
	pipeline_create_info.pMultisampleState = &multisample_state_create_info;
	pipeline_create_info.pDepthStencilState = &depth_stencil_info;
	pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
	pipeline_create_info.layout = _pipelineLayout[PipelineType::standard];
	pipeline_create_info.renderPass = _renderPass;
	pipeline_create_info.subpass = 0;
	pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;

	vk::tools::ErrorCheck(vkCreateGraphicsPipelines(_renderer->GetVulkanDevice(), VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &graphicsPipelines[PipelineType::standard]));

	rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_state_create_info.depthClampEnable = VK_FALSE;
	rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE; //for skipping rasterization
	rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_LINE; //FILL, LINE and POINT
	rasterization_state_create_info.lineWidth = 1.0f;
	rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterization_state_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization_state_create_info.depthBiasEnable = VK_FALSE; //used for shadow mapping

	pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_create_info.stageCount = 2;
	pipeline_create_info.pStages = shaderStages;
	pipeline_create_info.pVertexInputState = &vertex_input_info;
	pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
	pipeline_create_info.pViewportState = &viewport_state_create_info;
	pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
	pipeline_create_info.pMultisampleState = &multisample_state_create_info;
	pipeline_create_info.pDepthStencilState = &depth_stencil_info;
	pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
	pipeline_create_info.layout = _pipelineLayout[PipelineType::wireframe];
	pipeline_create_info.renderPass = _renderPass;
	pipeline_create_info.subpass = 0;
	pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;

	vk::tools::ErrorCheck(vkCreateGraphicsPipelines(_renderer->GetVulkanDevice(), VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &graphicsPipelines[PipelineType::wireframe]));

	vkDestroyShaderModule(_renderer->GetVulkanDevice(), vertShaderModule, nullptr);
	vkDestroyShaderModule(_renderer->GetVulkanDevice(), fragShaderModule, nullptr);
}

//Method for creating command pools for queues
void VulkanWindow::_CreateCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = _FindQueueFamilies(_renderer->GetVulkanPhysicalDevice());

	VkCommandPoolCreateInfo command_pool_create_info {};
	command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_create_info.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	vk::tools::ErrorCheck(vkCreateCommandPool(_renderer->GetVulkanDevice(), &command_pool_create_info, nullptr, &_commandPool));
}

//Method for creating command buffers from pools
void VulkanWindow::_CreateCommandBuffers()
{
}

//Method for creating semaphores
void VulkanWindow::_CreateSemaphores()
{
	_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphore_create_info = {};
	semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_create_info = {};
	fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vk::tools::ErrorCheck(vkCreateSemaphore(_renderer->GetVulkanDevice(), &semaphore_create_info, nullptr, &_imageAvailableSemaphores[i]));
		vk::tools::ErrorCheck(vkCreateSemaphore(_renderer->GetVulkanDevice(), &semaphore_create_info, nullptr, &_renderFinishedSemaphores[i]));
		vk::tools::ErrorCheck(vkCreateFence(_renderer->GetVulkanDevice(), &fence_create_info, nullptr, &_inFlightFences[i]));
	}
}

//Method for draw commands using command buffers
void VulkanWindow::DrawFrame()
{

	vkWaitForFences(_renderer->GetVulkanDevice(), 1, &_inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(_renderer->GetVulkanDevice(), _swapChain, std::numeric_limits<uint64_t>::max(),
		_imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		_RecreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

	_UpdateUniformBuffer(imageIndex);

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = waitSemaphores;
	submit_info.pWaitDstStageMask = waitStages;

	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &_drawCommandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[currentFrame] };
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signalSemaphores;
	//
	vkResetFences(_renderer->GetVulkanDevice(), 1, &_inFlightFences[currentFrame]);
	vk::tools::ErrorCheck(vkQueueSubmit(_renderer->GetVulkanGraphicsQueue(), 1, &submit_info, _inFlightFences[currentFrame]));

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { _swapChain };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapChains;
	present_info.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(_renderer->GetVulkanPresentQueue(), &present_info);
#if WIN32
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _frameBufferResized)
	{

		_frameBufferResized = false;
		_RecreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present swap chain image");
	}
#else
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _frameBufferResized)
	{

		_frameBufferResized = false;		
		_RecreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present swap chain image");
	}
#endif
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	//vkQueueWaitIdle(_renderer->GetVulkanPresentQueue());
}

//Method to create textured image
VkImage VulkanWindow::_CreateTextureImage(const char* path) 
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels) {
		throw std::runtime_error("Failed to load texture image!");
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	_CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(_renderer->GetVulkanDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(_renderer->GetVulkanDevice(), stagingBufferMemory);
	stbi_image_free(pixels);

	_CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _textureImage, _textureImageMemory);

	_TransitionImageLayout(_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	_CopyBufferToImage(stagingBuffer, _textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	_TransitionImageLayout(_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(_renderer->GetVulkanDevice(), stagingBuffer, nullptr);
	vkFreeMemory(_renderer->GetVulkanDevice(), stagingBufferMemory, nullptr);

	return _textureImage;
}

//Method to create a texture image view for the texture loaded
VkImageView VulkanWindow::_CreateTextureImageView(VkImage image)
{
	return _CreateImageView(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
}

//Method to create a new texture sampler
VkSampler VulkanWindow::_CreateTextureSampler()
{
	VkSampler sampler;
	VkSamplerCreateInfo sampler_info = {};
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter = VK_FILTER_LINEAR;
	sampler_info.minFilter = VK_FILTER_LINEAR;
	sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.anisotropyEnable = VK_FALSE;
	sampler_info.maxAnisotropy = 1;
	sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	sampler_info.unnormalizedCoordinates = VK_FALSE;
	sampler_info.compareEnable = VK_FALSE;
	sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
	sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler_info.mipLodBias = 0.0f;
	sampler_info.minLod = 0.0f;
	sampler_info.maxLod = 0.0f;

	vk::tools::ErrorCheck(vkCreateSampler(_renderer->GetVulkanDevice(), &sampler_info, nullptr, &sampler));
	return sampler;
}

//Method to create image
void VulkanWindow::_CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo image_create_info = {};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.extent.width = width;
	image_create_info.extent.height = height;
	image_create_info.extent.depth = 1;
	image_create_info.mipLevels = 1;
	image_create_info.arrayLayers = 1;
	image_create_info.format = format;
	image_create_info.tiling = tiling;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_create_info.usage = usage;
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;

	vk::tools::ErrorCheck(vkCreateImage(_renderer->GetVulkanDevice(), &image_create_info, nullptr, &image));

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(_renderer->GetVulkanDevice(), image, &memRequirements);

	VkMemoryAllocateInfo memory_alloc_info = {};
	memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_alloc_info.allocationSize = memRequirements.size;
	memory_alloc_info.memoryTypeIndex = _renderer->_GetMemoryType(memRequirements.memoryTypeBits, properties);

	vk::tools::ErrorCheck(vkAllocateMemory(_renderer->GetVulkanDevice(), &memory_alloc_info, nullptr, &imageMemory));

	vkBindImageMemory(_renderer->GetVulkanDevice(), image, imageMemory, 0);
}

//Method to create a single time use command buffer
VkCommandBuffer VulkanWindow::_BeginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandPool = _commandPool;
	alloc_info.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(_renderer->GetVulkanDevice(), &alloc_info, &commandBuffer);

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &begin_info);

	return commandBuffer;

}

//Method to end the single time command buffer
void VulkanWindow::_EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(_renderer->GetVulkanGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(_renderer->GetVulkanGraphicsQueue());

	vkFreeCommandBuffers(_renderer->GetVulkanDevice(), _commandPool, 1, &commandBuffer);
}

//Method to transition image layout
void VulkanWindow::_TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = _BeginSingleTimeCommands();
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (_HasStencilComponent(format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = 0;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		throw std::invalid_argument("Unsupported Layout Transition!");
	}

	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	_EndSingleTimeCommands(commandBuffer);
}

//Method to copy the buffer over to the image
void VulkanWindow::_CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = _BeginSingleTimeCommands();

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	_EndSingleTimeCommands(commandBuffer);

}

//Method to create an image view (Helper Method)
VkImageView VulkanWindow::_CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = format;
	view_info.components = {
		VK_COMPONENT_SWIZZLE_R,
		VK_COMPONENT_SWIZZLE_G,
		VK_COMPONENT_SWIZZLE_B,
		VK_COMPONENT_SWIZZLE_A
	};
	view_info.subresourceRange.aspectMask = aspectFlags;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;

	VkImageView imageView;
	vk::tools::ErrorCheck(vkCreateImageView(_renderer->GetVulkanDevice(), &view_info, nullptr, &imageView));

	return imageView;
}

//Method to create depth resources
void VulkanWindow::_CreateDepthResources()
{
	VkFormat depthFormat = _FindDepthFormat();

	_CreateImage(_swapChainExtent.width, _swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depthImages, _depthImagesMemory);
	_depthImagesView = _CreateImageView(_depthImages, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	_TransitionImageLayout(_depthImages, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

}

//Method to set up supported formats
VkFormat VulkanWindow::_FindSupportFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(_renderer->GetVulkanPhysicalDevice(), format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	throw std::runtime_error("Failed to find supported format!");

}

//Method to find the depth format
VkFormat VulkanWindow::_FindDepthFormat()
{
	return _FindSupportFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

//Method to check if format has the stencil component
bool VulkanWindow::_HasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

//Method to update GLFW window
void VulkanWindow::Update()
{
	//Update glfw window if the window is open
	while (!glfwWindowShouldClose(_window))
	{
		glfwPollEvents();
		DrawFrame();
	}

	vkDeviceWaitIdle(_renderer->GetVulkanDevice());
}

//Method to create a surface variable for glfw and vulkan support
void VulkanWindow::_InitSurface()
{
#ifdef WIN32
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfoKHR = {};
	surfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfoKHR.hwnd = _win32_window;
	surfaceCreateInfoKHR.hinstance = GetModuleHandle(nullptr);

	vk::tools::ErrorCheck(vkCreateWin32SurfaceKHR(_renderer->GetVulkanInstance(), &surfaceCreateInfoKHR, nullptr, &_surface));
	//std::cout << _win32_window << std::endl;

	//vk::tools::ErrorCheck(glfwCreateWindowSurface(_renderer->GetVulkanInstance(), _window, nullptr, &_surface));
#else
	vk::tools::ErrorCheck(glfwCreateWindowSurface(_renderer->GetVulkanInstance(), _window, nullptr, &_surface));
#endif
}

//Method to destroy the surface variable for glfw and vulkan support
void VulkanWindow::_DeInitSurface()
{
	vkDestroySurfaceKHR(_renderer->GetVulkanInstance(), _surface, nullptr);
}

//Method to initialise GLFW window
void VulkanWindow::_InitWindow(int width, int height)
{
	
#if WIN32

#else
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	_window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(_window, this);
	glfwSetFramebufferSizeCallback(_window, _FrameBufferResizeCallback);
#endif

}

//Method to create swap chain with surface KHR
void VulkanWindow::_CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = _QuerySwapChainSupport(_renderer->GetVulkanPhysicalDevice());

	VkSurfaceFormatKHR surfaceFormat = _ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = _ChooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = _ChooseSwapExtent(swapChainSupport.capabilities);


	//if the value of the maxImageCount is 0 then there is no limit to image count
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		//if the max image count has a limit, set it to the maxImageCount
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swap_chain_create_info = {};
	swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swap_chain_create_info.surface = _surface;
	swap_chain_create_info.minImageCount = imageCount;
	swap_chain_create_info.imageFormat = surfaceFormat.format;
	swap_chain_create_info.imageColorSpace = surfaceFormat.colorSpace;
	swap_chain_create_info.imageExtent = extent;

	//specifies the amount of layers each image can consist of (always 1 unless steroscopic 3D)
	swap_chain_create_info.imageArrayLayers = 1;

	//what kind of operations the image in the swap chain are for (come back to when post processing)
	//if post processing would use VK_IMAGE_USAGE_TRANSFER_DST_BIT and use memory operation to transfer image
	swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = _FindQueueFamilies(_renderer->GetVulkanPhysicalDevice());
	uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily};

	if (indices.graphicsFamily != indices.presentFamily)
	{
		swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swap_chain_create_info.queueFamilyIndexCount = 2;
		swap_chain_create_info.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	//*VK_SHARING_MODE_EXCLUSIVE = image is ownd by one family at a time and ownership 
	//							   must be transfered
	//*VK_SHARING_MODE_CONCURRENT = Images can be used across mltiple queue families
	//							   without ownership transfer

	//any transforms (90 degree flips or anything for post processing)
	swap_chain_create_info.preTransform = swapChainSupport.capabilities.currentTransform;
	swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swap_chain_create_info.presentMode = presentMode;

	//if window is ontop and pixels are obscured then don't read those pixels
	swap_chain_create_info.clipped = VK_TRUE;

	//if swapchain becomes invalid or unoptimised while running you can
	//recreate the swapchain or reference an old one 
	swap_chain_create_info.oldSwapchain = VK_NULL_HANDLE;

	vk::tools::ErrorCheck(vkCreateSwapchainKHR(_renderer->GetVulkanDevice(), &swap_chain_create_info, nullptr, &_swapChain));

	vkGetSwapchainImagesKHR(_renderer->GetVulkanDevice(), _swapChain, &imageCount, nullptr);
	_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(_renderer->GetVulkanDevice(), _swapChain, &imageCount, _swapChainImages.data());

	_swapChainImageFormat = surfaceFormat.format;
	_swapChainExtent = extent;
}

//Method to create image views
void VulkanWindow::_CreateImageViews()
{
	//resize the swap chain image views to the amount of swapchain images
	_swapChainImageViews.resize(_swapChainImages.size());

	//loop through swapchain images
	for (size_t i = 0; i < _swapChainImages.size(); i++)
	{
		_swapChainImageViews[i] = _CreateImageView(_swapChainImages[i], _swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

//Method for creating frame buffers
void VulkanWindow::_CreateFramebuffers()
{
	_swapChainFramebuffers.resize(_swapChainImageViews.size());

	for (size_t i = 0; i < _swapChainImageViews.size(); i++)
	{
		std::array<VkImageView, 2> attachments = {
			_swapChainImageViews[i],
			_depthImagesView
		};

		VkFramebufferCreateInfo framebuffer_create_info = {};
		framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_create_info.pNext = 0;
		framebuffer_create_info.renderPass = _renderPass;
		framebuffer_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebuffer_create_info.pAttachments = attachments.data();
		framebuffer_create_info.width = _swapChainExtent.width;
		framebuffer_create_info.height = _swapChainExtent.height;
		framebuffer_create_info.layers = 1;

		vk::tools::ErrorCheck(vkCreateFramebuffer(_renderer->GetVulkanDevice(), &framebuffer_create_info, nullptr, &_swapChainFramebuffers[i]));
	}
}

//Method for recreation of swapchain incase of VKResult error
void VulkanWindow::_RecreateSwapChain()
{
	int width = 0, height = 0;
	while (width == 0 || height == 0)
	{
#ifdef WIN32
		width = 1024;
		height = 768;
#else
		width = 
		glfwWaitEvents();
#endif // WIN32

		
	}


	vkDeviceWaitIdle(_renderer->GetVulkanDevice());

	_CleanUpSwapChain();

	VulkanWindow::_CreateSwapChain();
	VulkanWindow::_CreateImageViews();
	//VulkanWindow::_CreateRenderPass();
	//VulkanWindow::_CreateGraphicsPipeline();
	VulkanWindow::_CreateDepthResources();
	VulkanWindow::_CreateFramebuffers();
	VulkanWindow::_CreateCommandBuffers();
}

//Creates an attachment for a framebuffer and framebuffer image view
void VulkanWindow::_CreateAttachment(VkFormat format, VkImageUsageFlagBits usageFlags, vk::wrappers::FrameBufferAttachment * frameBufferAttachment, vk::wrappers::GFrameBuffer frameBuffer)
{
	//Initialise local variables
	VkImageAspectFlags aspectMask = 0;
	VkImageLayout imageLayout;

	//Set the attachment format
	frameBufferAttachment->format = format;

	//if the attachment we're using is not the depth test
	if (usageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
	{
		//set the aspect mask to image color bit
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	//if the attachment we're using is the depth test
	if (usageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		//set the depth test mask and image stencil flags and change image layout to depth stencil optimal
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	//make sure the aspect mask is greater than 0 (Depth test or image color attachment flags have been met)
	assert(aspectMask > 0);

	//Set up create info for the image for the framebuffer using format, and framebuffer we have
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.extent.width = frameBuffer.width;
	imageCreateInfo.extent.height = frameBuffer.height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = usageFlags | VK_IMAGE_USAGE_SAMPLED_BIT;

	//Initialise mem alloc and mem requirement variables
	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;

	//Create the image for the framebuffer and check the memory requirements to bind and allocate memory for the image
	vk::tools::ErrorCheck(vkCreateImage(_renderer->GetVulkanDevice(), &imageCreateInfo, nullptr, &frameBufferAttachment->image));
	vkGetImageMemoryRequirements(_renderer->GetVulkanDevice(), frameBufferAttachment->image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = _renderer->_GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vk::tools::ErrorCheck(vkAllocateMemory(_renderer->GetVulkanDevice(), &memAlloc, nullptr, &frameBufferAttachment->mem));
	vk::tools::ErrorCheck(vkBindImageMemory(_renderer->GetVulkanDevice(), frameBufferAttachment->image, frameBufferAttachment->mem, 0));

	//Set up image view create info to create the view for the image buffer
	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.components = {
		VK_COMPONENT_SWIZZLE_R,
		VK_COMPONENT_SWIZZLE_G,
		VK_COMPONENT_SWIZZLE_B,
		VK_COMPONENT_SWIZZLE_A
	};
	imageViewCreateInfo.subresourceRange = {};
	imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	imageViewCreateInfo.image = frameBufferAttachment->image;

	//Create the image view for the frameBufferAttachment
	vk::tools::ErrorCheck(vkCreateImageView(_renderer->GetVulkanDevice(), &imageViewCreateInfo, nullptr, &frameBufferAttachment->view));


}

//Creates an attachment for a framebuffer and framebuffer image view
void VulkanWindow::_CreateAttachment(VkFormat format, VkImageUsageFlagBits usageFlags, vk::wrappers::FrameBufferAttachment * frameBufferAttachment, vk::wrappers::VFrameBuffer frameBuffer)
{
	//Initialise local variables
	VkImageAspectFlags aspectMask = 0;
	VkImageLayout imageLayout;

	//Set the attachment format
	frameBufferAttachment->format = format;

	//if the attachment we're using is not the depth test
	if (usageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
	{
		//set the aspect mask to image color bit
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	//if the attachment we're using is the depth test
	if (usageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		//set the depth test mask and image stencil flags and change image layout to depth stencil optimal
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	//make sure the aspect mask is greater than 0 (Depth test or image color attachment flags have been met)
	assert(aspectMask > 0);

	//Set up create info for the image for the framebuffer using format, and framebuffer we have
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.extent.width = frameBuffer.width;
	imageCreateInfo.extent.height = frameBuffer.height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = usageFlags | VK_IMAGE_USAGE_SAMPLED_BIT;

	//Initialise mem alloc and mem requirement variables
	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;

	//Create the image for the framebuffer and check the memory requirements to bind and allocate memory for the image
	vk::tools::ErrorCheck(vkCreateImage(_renderer->GetVulkanDevice(), &imageCreateInfo, nullptr, &frameBufferAttachment->image));
	vkGetImageMemoryRequirements(_renderer->GetVulkanDevice(), frameBufferAttachment->image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = _renderer->_GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vk::tools::ErrorCheck(vkAllocateMemory(_renderer->GetVulkanDevice(), &memAlloc, nullptr, &frameBufferAttachment->mem));
	vk::tools::ErrorCheck(vkBindImageMemory(_renderer->GetVulkanDevice(), frameBufferAttachment->image, frameBufferAttachment->mem, 0));

	//Set up image view create info to create the view for the image buffer
	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.components = {
		VK_COMPONENT_SWIZZLE_R,
		VK_COMPONENT_SWIZZLE_G,
		VK_COMPONENT_SWIZZLE_B,
		VK_COMPONENT_SWIZZLE_A
	};
	imageViewCreateInfo.subresourceRange = {};
	imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	imageViewCreateInfo.image = frameBufferAttachment->image;

	//Create the image view for the frameBufferAttachment
	vk::tools::ErrorCheck(vkCreateImageView(_renderer->GetVulkanDevice(), &imageViewCreateInfo, nullptr, &frameBufferAttachment->view));


}

//Creates an attachment for a framebuffer and framebuffer image view
void VulkanWindow::_CreateAttachment(VkFormat format, VkImageUsageFlagBits usageFlags, vk::wrappers::FrameBufferAttachment * frameBufferAttachment, vk::wrappers::ShadowFrameBuffer frameBuffer)
{
	//Initialise local variables
	VkImageAspectFlags aspectMask = 0;
	VkImageLayout imageLayout;

	//Set the attachment format
	frameBufferAttachment->format = format;

	//if the attachment we're using is not the depth test
	if (usageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
	{
		//set the aspect mask to image color bit
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	//if the attachment we're using is the depth test
	if (usageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		//set the depth test mask and image stencil flags and change image layout to depth stencil optimal
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	//make sure the aspect mask is greater than 0 (Depth test or image color attachment flags have been met)
	assert(aspectMask > 0);

	//Set up create info for the image for the framebuffer using format, and framebuffer we have
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.extent.width = frameBuffer.width;
	imageCreateInfo.extent.height = frameBuffer.height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = usageFlags | VK_IMAGE_USAGE_SAMPLED_BIT;

	//Initialise mem alloc and mem requirement variables
	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;

	//Create the image for the framebuffer and check the memory requirements to bind and allocate memory for the image
	vk::tools::ErrorCheck(vkCreateImage(_renderer->GetVulkanDevice(), &imageCreateInfo, nullptr, &frameBufferAttachment->image));
	vkGetImageMemoryRequirements(_renderer->GetVulkanDevice(), frameBufferAttachment->image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = _renderer->_GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vk::tools::ErrorCheck(vkAllocateMemory(_renderer->GetVulkanDevice(), &memAlloc, nullptr, &frameBufferAttachment->mem));
	vk::tools::ErrorCheck(vkBindImageMemory(_renderer->GetVulkanDevice(), frameBufferAttachment->image, frameBufferAttachment->mem, 0));

	//Set up image view create info to create the view for the image buffer
	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.components = {
		VK_COMPONENT_SWIZZLE_R,
		VK_COMPONENT_SWIZZLE_G,
		VK_COMPONENT_SWIZZLE_B,
		VK_COMPONENT_SWIZZLE_A
	};
	imageViewCreateInfo.subresourceRange = {};
	imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	imageViewCreateInfo.image = frameBufferAttachment->image;

	//Create the image view for the frameBufferAttachment
	vk::tools::ErrorCheck(vkCreateImageView(_renderer->GetVulkanDevice(), &imageViewCreateInfo, nullptr, &frameBufferAttachment->view));


}


//Method for creating a VkBuffer
void VulkanWindow::_CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer & buffer, VkDeviceMemory & bufferMemory)
{
	VkBufferCreateInfo buffer_create_info = {};
	buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.size = size;
	buffer_create_info.usage = usage;
	buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	vk::tools::ErrorCheck(vkCreateBuffer(_renderer->GetVulkanDevice(), &buffer_create_info, nullptr, &buffer));

	VkMemoryRequirements mem_requirements;
	vkGetBufferMemoryRequirements(_renderer->GetVulkanDevice(), buffer, &mem_requirements);

	VkMemoryAllocateInfo memory_allocate_info = {};
	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize = mem_requirements.size;
	memory_allocate_info.memoryTypeIndex = _renderer->_GetMemoryType(mem_requirements.memoryTypeBits, properties);

	vk::tools::ErrorCheck(vkAllocateMemory(_renderer->GetVulkanDevice(), &memory_allocate_info, nullptr, &bufferMemory));

	vkBindBufferMemory(_renderer->GetVulkanDevice(), buffer, bufferMemory, 0);
}

//Method to copy the buffer to VRAM
void VulkanWindow::_CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = _BeginSingleTimeCommands();

	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	_EndSingleTimeCommands(commandBuffer);
}

//Method to create a uniform buffer 
void VulkanWindow::_CreateUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	_uniformBuffers.resize(_swapChainImages.size());
	_uniformBuffersMemory.resize(_swapChainImages.size());

	for (size_t i = 0; i < _swapChainImages.size(); i++)
	{
		_CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _uniformBuffers[i], _uniformBuffersMemory[i]);
	}
}

//Method to create a descriptor pool
void VulkanWindow::_CreateDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> pool_sizes = {};
	pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_sizes[0].descriptorCount = static_cast<uint32_t>(_swapChainImages.size());
	pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_sizes[1].descriptorCount = static_cast<uint32_t>(_swapChainImages.size());


	VkDescriptorPoolCreateInfo pool_create_info = {};
	pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
	pool_create_info.pPoolSizes = pool_sizes.data();
	pool_create_info.maxSets = static_cast<uint32_t>(_swapChainImages.size());

	vk::tools::ErrorCheck(vkCreateDescriptorPool(_renderer->GetVulkanDevice(), &pool_create_info, nullptr, &_descriptorPool));
}

//Method to create a descriptor set(s) 
void VulkanWindow::_CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(_swapChainImages.size(), _descriptorSetLayout);

	VkDescriptorSetAllocateInfo descriptor_alloc_info = {};
	descriptor_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptor_alloc_info.descriptorPool = _descriptorPool;
	descriptor_alloc_info.descriptorSetCount = static_cast<uint32_t>(_swapChainImages.size());
	descriptor_alloc_info.pSetLayouts = layouts.data();

	_descriptorSets.resize(_swapChainImages.size());

	vk::tools::ErrorCheck(vkAllocateDescriptorSets(_renderer->GetVulkanDevice(), &descriptor_alloc_info, _descriptorSets.data()));

	for (size_t i = 0; i < _swapChainImages.size(); i++)
	{
		VkDescriptorBufferInfo buffer_info = {};
		buffer_info.buffer = _uniformBuffers[i];
		buffer_info.offset = 0;
		buffer_info.range = sizeof(UniformBufferObject);
		
		VkDescriptorImageInfo image_info = {};
		image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_info.imageView = _textureImageView;
		image_info.sampler = _textureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptor_writes = {};
		descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[0].dstSet = _descriptorSets[i];
		descriptor_writes[0].dstBinding = 0;
		descriptor_writes[0].dstArrayElement = 0;
		descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_writes[0].descriptorCount = 1;
		descriptor_writes[0].pBufferInfo = &buffer_info;

		descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[1].dstSet = _descriptorSets[i];
		descriptor_writes[1].dstBinding = 1;
		descriptor_writes[1].dstArrayElement = 0;
		descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_writes[1].descriptorCount = 1;
		descriptor_writes[1].pImageInfo = &image_info;

		vkUpdateDescriptorSets(_renderer->GetVulkanDevice(), static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
	}

	

}

//Method for Creating the descriptor set layout (BASE : Forward Rendering Descriptor for shader)
void VulkanWindow::_CreateDescriptorSetLayout()
{

	//UBO layout binding
	VkDescriptorSetLayoutBinding ubo_layout_binding = {};
	ubo_layout_binding.binding = 0;
	ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	ubo_layout_binding.pImmutableSamplers = nullptr;

	//Texture Sampler layout binding
	VkDescriptorSetLayoutBinding sampler_layout_binding = {};
	sampler_layout_binding.binding = 1;
	sampler_layout_binding.descriptorCount = 1;
	sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sampler_layout_binding.pImmutableSamplers = nullptr;
	sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	//Store bindings and hand them to the descriptor set layout
	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { ubo_layout_binding, sampler_layout_binding };
	VkDescriptorSetLayoutCreateInfo layout_info = {};
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
	layout_info.pBindings = bindings.data();

	vk::tools::ErrorCheck(vkCreateDescriptorSetLayout(_renderer->GetVulkanDevice(), &layout_info, nullptr, &_descriptorSetLayout));
}

//Method to update the data in the uniform buffer
void VulkanWindow::_UpdateUniformBuffer(uint32_t imageIndex)
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo = {};
	//ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	ubo.view = glm::lookAt(glm::vec3(2.0f, 6.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	ubo.proj = glm::perspective(glm::radians(45.0f), _swapChainExtent.width / (float)_swapChainExtent.height, 0.1f, 20.0f);
	
	//Usually made for OpenGL where Y coordinate of the clip coordinates are invertex, this flips back
	ubo.proj[1][1] *= -1;
	

	void* data;
	vkMapMemory(_renderer->GetVulkanDevice(), _uniformBuffersMemory[imageIndex], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(_renderer->GetVulkanDevice(), _uniformBuffersMemory[imageIndex]);

}

//Method to destroy framebuffers
void VulkanWindow::_DeInitFramebuffers()
{
	for (auto framebuffer : _swapChainFramebuffers)
	{
		vkDestroyFramebuffer(_renderer->GetVulkanDevice(), framebuffer, nullptr);
	}
}

//Method to destroy swap chain
void VulkanWindow::_DeInitSwapChain()
{
	vkDestroySwapchainKHR(_renderer->GetVulkanDevice(), _swapChain, nullptr);
}

//Method to destroy image views
void VulkanWindow::_DeInitImageViews()
{
	//For each image view
	for (auto imageView : _swapChainImageViews)
	{
		//Destroy the image view
		vkDestroyImageView(_renderer->GetVulkanDevice(), imageView, nullptr);
	}
}

//Method to destroy semaphores
void VulkanWindow::_DeInitSemaphores()
{
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(_renderer->GetVulkanDevice(), _renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(_renderer->GetVulkanDevice(), _imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(_renderer->GetVulkanDevice(), _inFlightFences[i], nullptr);
	}
}

//Method to destroy render pass
void VulkanWindow::_DeInitRenderPass()
{
	vkDestroyRenderPass(_renderer->GetVulkanDevice(), _renderPass, nullptr);
}

//Method to destroy pipeline layout
void VulkanWindow::_DeInitPipelineLayout()
{
	for (auto i : _pipelineLayout)
	{
		vkDestroyPipelineLayout(_renderer->GetVulkanDevice(), i.second, nullptr);
	}
}

//Method to destroy graphics pipeline
void VulkanWindow::_DeInitGraphicsPipeline()
{
	for (auto i : graphicsPipelines)
	{
		vkDestroyPipeline(_renderer->GetVulkanDevice(), i.second, nullptr);	
	}

	vkDestroyPipeline(_renderer->GetVulkanDevice(), _graphicsPipeline, nullptr);
}

//Method to destroy GLFW window
void VulkanWindow::_DeInitWindow()
{
#ifdef WIN32

#else
	glfwDestroyWindow(_window);

	glfwTerminate();
#endif

}

//Method to destroy command pool
void VulkanWindow::_DeInitCommandPool()
{
	vkDestroyCommandPool(_renderer->GetVulkanDevice(), _commandPool, nullptr);
}

//Method to quick cleanup swapchain before recreation and applicaton close
void VulkanWindow::_CleanUpSwapChain()
{

	vkDestroyImageView(_renderer->GetVulkanDevice(), _depthImagesView, nullptr);
	vkDestroyImage(_renderer->GetVulkanDevice(), _depthImages, nullptr);
	vkFreeMemory(_renderer->GetVulkanDevice(), _depthImagesMemory, nullptr);
	


	_DeInitFramebuffers();
	_DeInitGraphicsPipeline();
	_DeInitPipelineLayout();
	_DeInitRenderPass();
	_DeInitImageViews();
	_DeInitSwapChain();
}