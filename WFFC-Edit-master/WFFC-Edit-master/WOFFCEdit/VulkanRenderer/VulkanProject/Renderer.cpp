#define NOMINMAX
#include "Renderer.h"
#include "Shared.h"
#include <iostream>
#include <cstdlib>
#include <assert.h>
#include <vector>
#include <sstream>
#include <set>
#include <algorithm>

//Get a reference of the functions that do not exist inside vulkancore.h
PFN_vkCreateDebugReportCallbackEXT	fvkCreateDebugReportCallbackEXT = nullptr;
PFN_vkDestroyDebugReportCallbackEXT	fvkDestroyDebugReportCallbackEXT = nullptr;

//constructor
Renderer::Renderer()
{
#ifdef WIN32
#else
	glfwInit();
#endif
	_SetupDebug(); 
	_InitInstance();
	_InitDebug();
	_InitDevice();
}

//deconstructor
Renderer::~Renderer()
{
	_DeInitDevice();
	_DeInitDebug();
	_DeInitInstance();
}

//creates an instance of vulkan api
void Renderer::_InitInstance()
{

	//checking available extentions for debug purposes:
	uint32_t extention_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extention_count, nullptr);
	std::vector<VkExtensionProperties> extentions(extention_count);
	vkEnumerateInstanceExtensionProperties(nullptr, &extention_count, extentions.data());

	//output the available instance extentions to the console window
	for (auto i = 0; i < extentions.size(); i++)
	{
		std::cout << "Available Extentions: " << extentions[i].extensionName << std::endl;
	}
	std::cout << std::endl;

	//_instance_extentions.push_back("");


	//add glfw extentions to extention list
	glfwExtentions = glfwGetRequiredInstanceExtensions(&glfwExtentionCount);
	for (auto i = 0; i < glfwExtentionCount; i++)
	{

		_instance_extentions.push_back(glfwExtentions[i]);
		std::cout << "GLFW EXTENTION ADDED: " << glfwExtentions[i] << std::endl;
	}
	std::cout << std::endl;


	//set up the application information for the instance (API version, program application version and name)
	VkApplicationInfo application_info{};
	application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.apiVersion = VK_API_VERSION_1_0;
	application_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	application_info.pApplicationName = "Vulkan Tutorial 1";

	//Set up the instance create information (Validation layers, application layers, extention counts)
	VkInstanceCreateInfo instance_create_info{};
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.pApplicationInfo = &application_info;
	instance_create_info.enabledLayerCount = _instance_layers.size();
	instance_create_info.ppEnabledLayerNames = _instance_layers.data();
	instance_create_info.enabledExtensionCount = _instance_extentions.size();
	instance_create_info.ppEnabledExtensionNames = _instance_extentions.data();
	instance_create_info.pNext = &debug_callback_create_info;

	//Create the instance using vkCreateInstance function, and error check
	vk::tools::ErrorCheck(vkCreateInstance(&instance_create_info, nullptr, &_instance));

}

//Destroy instance of vulkan api
void Renderer::_DeInitInstance()
{
	//destroy's the Vulkan instance
	vkDestroyInstance(_instance, nullptr);
	_instance = nullptr;


}

//Method to check if a device is suitable for 3D application (graphics bit and presentation bit)
bool Renderer::_IsDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = _FindQueueFamilies(device);

	bool extensionsSupported = _CheckDeviceExtentionSupport(device);

	return indices.isComplete() && extensionsSupported;
}

//Method to check device extention support (lists and loops through)
bool Renderer::_CheckDeviceExtentionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());


	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

//Method to find and filter the queue families of the device we have chosen
QueueFamilyIndices  Renderer::_FindQueueFamilies(VkPhysicalDevice device) {
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
			indices.presentFamily = i;
		}

		//query the physical device for surface support
		//VkBool32 presentSupport = VK_NULL_HANDLE;
		//vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);

		//if suitable
		if (indices.isComplete()) {
			//break out of the loop and return indices
			break;
		}

		i++;
	}

	return indices;
}

//API call for debugcallbacks
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT obj_type, uint64_t src_obj, size_t location, int32_t msg_code, const char* layer_prefix, const char* msg, void * user_data)
{
	std::ostringstream stream;
	stream << "VKDBG: ";
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
	{
		stream << "INFO: ";
	}
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		stream << "WARNING: ";
	}
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		stream << "PERFORMANCE: ";
	}
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		stream << "ERROR: ";
	}
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
	{
		stream << "DEBUG: ";
	}
	stream << "@[" << layer_prefix << "]: ";
	stream << msg << std::endl;
	std::cout << stream.str();
	#ifdef _WIN32
		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		{
			//MessageBox(NULL, stream.str().c_str(), "Vulkan ERROR!", 0);
		}
	#endif // _WIN32
	return false;
}

//Initialises device based on available hardware
void Renderer::_InitDevice()
{
	{
		//get the number of GPU devices on the computer
		uint32_t gpu_count = 0;

		//call once to get the GPU count
		vkEnumeratePhysicalDevices(_instance, &gpu_count, nullptr);

		//store count
		std::vector<VkPhysicalDevice> gpu_list(gpu_count);

		//call again to get a list of the GPUs
		vkEnumeratePhysicalDevices(_instance, &gpu_count, gpu_list.data());

		//Find out if a device is suitable to be chosen for our project
		for (const auto& device : gpu_list)
		{
			if (_IsDeviceSuitable(device))
			{
				_gpu = device;
				break;
			}
			else
			{
				assert("GPU NOT SUITABLE FOR THIS PROJECT");
			}
		}

		//get the physical device properties of the GPU we're accessing
		vkGetPhysicalDeviceProperties(_gpu, &_gpu_properties);
		vkGetPhysicalDeviceMemoryProperties(_gpu, &_gpu_memory_properties);
		
	}



	{
		//get the number of layers from the instance
		uint32_t layer_count = 0;

		//do once to get the layer count
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

		//create a vector with the size of layers the instance has
		std::vector<VkLayerProperties> layer_property_List(layer_count);

		//call twice to store the layer instance amount
		vkEnumerateInstanceLayerProperties(&layer_count, layer_property_List.data());

		//write out all layers found using iostream
		std::cout << "Instance Layers: \n";
		for (auto &i : layer_property_List)
		{
			std::cout << "	" << i.layerName << "\t\t | " << i.description << std::endl;
		}
		std::cout << std::endl;

	}

	_indices = _FindQueueFamilies(_gpu);
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = { _indices.graphicsFamily, _indices.presentFamily };

	//QUEUE DEVICE INFO FOR THE GRAPHICS FAMILY QUEUE WE ARE INTERSTED IN
	float queue_priorities[]{ 1.0f };
	for (int queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo device_queue_create_info{};
		device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		device_queue_create_info.queueFamilyIndex = queueFamily;
		device_queue_create_info.queueCount = 1;
		device_queue_create_info.pQueuePriorities = queue_priorities;
		queueCreateInfos.push_back(device_queue_create_info);
	}


	//Device info for the device we are creating 
	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	device_create_info.pQueueCreateInfos = queueCreateInfos.data();
	device_create_info.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	device_create_info.ppEnabledExtensionNames = deviceExtensions.data();

	//Create a device using the vkCreateDevice function, and error check that we have a device 
	vk::tools::ErrorCheck(vkCreateDevice(_gpu, &device_create_info, nullptr, &_device));
	
	std::cout << "==GPU Information==" << std::endl;
	std::cout << std::endl;
	std::cout << "Device Name: " << _gpu_properties.deviceName << std::endl;
	std::cout << "Device Type: " << _gpu_properties.deviceType << std::endl;
	std::cout << "Max per stage descriptor storage buffers: "<< _gpu_properties.limits.maxPerStageDescriptorStorageBuffers << std::endl;
	std::cout << "Max Set Descriptor storage buffers: " <<_gpu_properties.limits.maxDescriptorSetStorageBuffers << std::endl;
	std::cout << "Max Set Descriptor dynamic storage buffers: " << _gpu_properties.limits.maxDescriptorSetStorageBuffersDynamic << std::endl;
	std::cout << "Max Set Descriptor uniform buffers: " << _gpu_properties.limits.maxDescriptorSetUniformBuffers << std::endl;
	std::cout << "Max Set Descriptor dynamic uniform buffers: " << _gpu_properties.limits.maxDescriptorSetUniformBuffersDynamic << std::endl;
	std::cout << std::endl;;
	vkGetDeviceQueue(_device, _indices.graphicsFamily, 0, &_graphicsQueue);
	vkGetDeviceQueue(_device, _indices.presentFamily, 0, &_presentQueue);
}

//DeInit Device (unload)
void Renderer::_DeInitDevice()
{
	vkDestroyDevice(_device, nullptr);
	_device = VK_NULL_HANDLE;
}

//Method to find the memory is suitable for the type of the GPU 
uint32_t Renderer::_GetMemoryType(uint32_t memTypeBits, VkMemoryPropertyFlags propertyFlags, VkBool32 * memTypeFound)
{
	//for each of the memory types that the GPU has
	for (uint32_t i = 0; i < _gpu_memory_properties.memoryTypeCount; i++)
	{
		//Check the memTypes has flags
		if ((memTypeBits & 1) == 1)
		{
			//if the property flags are the same
			if ((_gpu_memory_properties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
			{
				//found the memory type
				if (memTypeFound)
				{
					*memTypeFound = true;
				}
				return i;
			}
		}
		memTypeBits >>= 1;
	}

	if (memTypeFound)
	{
		*memTypeFound = false;
	}
	else
	{
		throw std::runtime_error("Could not find a matching memory type");
	}
}

//Debug setup
void Renderer::_SetupDebug()
{
	//Set up the debug callback information structure with the API call function and flags
	debug_callback_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	debug_callback_create_info.pfnCallback = VulkanDebugCallback;
	debug_callback_create_info.flags =
		//VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_DEBUG_BIT_EXT |
		0;

	//pushback the layer we want to use as our validation layers for the instance
	_instance_layers.push_back("VK_LAYER_RENDERDOC_Capture");
	_instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");
	_instance_layers.push_back("VK_LAYER_NV_optimus");
	//pushback the instance extention debug reports
	_instance_extentions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	_instance_extentions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	_instance_extentions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
}

//Method to Initialise the debug callback methods of validation layers 
void Renderer::_InitDebug()
{
	//Create a function reference of the createdebuginstancecallbackext function
	fvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT");
	fvkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugReportCallbackEXT");

	if (fvkCreateDebugReportCallbackEXT == nullptr || fvkDestroyDebugReportCallbackEXT == nullptr)
	{
		assert(0 && "Vulkan ERROR: Can't fetch debug function pointer");
		std::exit(-1);
	}


	fvkCreateDebugReportCallbackEXT(_instance, &debug_callback_create_info, nullptr, &_debug_report);
}

//Deinitialise the debug callback extentions
void Renderer::_DeInitDebug()
{
	fvkDestroyDebugReportCallbackEXT(_instance, _debug_report, nullptr);
	_debug_report = VK_NULL_HANDLE;
}