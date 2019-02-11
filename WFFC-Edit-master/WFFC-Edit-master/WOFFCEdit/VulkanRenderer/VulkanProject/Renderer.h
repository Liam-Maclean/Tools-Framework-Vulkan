#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <glm/glm.hpp>
#include <array>
//Queue family indices (checks if the queue indices are all handled)
struct QueueFamilyIndices {
	int graphicsFamily = -1;
	int presentFamily = -1;
	bool isComplete()
	{
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};

class Renderer
{
public:

	Renderer();
	~Renderer();

	//==Instance==
	void _InitInstance();
	void _DeInitInstance();

	//==Device==
	void _InitDevice();
	void _DeInitDevice();
	uint32_t _GetMemoryType(uint32_t memTypeBits, VkMemoryPropertyFlags propertyFlags, VkBool32 *memTypeFound = nullptr);
	QueueFamilyIndices _FindQueueFamilies(VkPhysicalDevice device);
	bool _IsDeviceSuitable(VkPhysicalDevice device);
	bool _CheckDeviceExtentionSupport(VkPhysicalDevice device);

	//==Debug==
	void _SetupDebug();
	void _InitDebug();
	void _DeInitDebug();

	//Surface
	VkSurfaceKHR _surface;

	//==Getters==
	const VkInstance GetVulkanInstance() const
	{
		return _instance;
	}
	VkDevice GetVulkanDevice() 
	{
		return _device;
	}
	const VkPhysicalDevice GetVulkanPhysicalDevice() const
	{
		return _gpu;
	}

	const VkQueue GetVulkanGraphicsQueue() const
	{
		return _graphicsQueue;
	}

	const VkQueue GetVulkanPresentQueue() const
	{
		return _presentQueue;
	}

	const uint32_t GetVulkanGraphicsQueueFamilyIndex() const
	{
		return _indices.graphicsFamily;
	}
	const VkPhysicalDeviceProperties & GetVulkanPhysicalDeviceProperties() const
	{
		return _gpu_properties;
	}
	const VkPhysicalDeviceMemoryProperties &GetVulkanPhysicalDeviceMemoryProperties() const
	{
		return _gpu_memory_properties;
	}


	//debug extention methods
	VkDebugReportCallbackCreateInfoEXT debug_callback_create_info = {};
	VkDebugReportCallbackEXT _debug_report = VK_NULL_HANDLE;

	//Device extentions
#ifdef WIN32
	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
#else
	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
#endif
	//Instance, device and queues
	QueueFamilyIndices _indices;
	VkInstance _instance = VK_NULL_HANDLE;
	VkPhysicalDevice _gpu = VK_NULL_HANDLE;
	VkDevice _device = VK_NULL_HANDLE;
	VkQueue _graphicsQueue = VK_NULL_HANDLE;
	VkQueue _presentQueue = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties _gpu_properties = {};
	VkPhysicalDeviceMemoryProperties _gpu_memory_properties = {};

	//Instance layers and extentions
	std::vector<const char*> _instance_layers;
	std::vector<const char*> _instance_extentions = {
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	};

	//glfw extentions
	const char** glfwExtentions;
	uint32_t glfwExtentionCount = 0;
};

