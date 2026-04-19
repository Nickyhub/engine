#include "vulkan_instance.h"
#include "vulkan_utils.h"
#include "vulkan_types.inl"

#include "defines.h"
#include "core/logger.h"
#include "core/platform.h"
#include "core/estring.h"
#include "containers/darray.h"
#include "memory/ememory.h"
#include <vulkan/vulkan_win32.h>

b8 create_debug_messenger(const vulkan_instance *instance, VkDebugUtilsMessengerEXT *debug_messenger);

// Callback for the Debug Messenger for validation layer errors
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSevertiy,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	void *pUserData)
{
	if (messageSevertiy & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		EN_ERROR("%s", pCallbackData->pMessage);
	}
	if (messageSevertiy & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		EN_WARN("%s", pCallbackData->pMessage);
	}
	if (messageSevertiy & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
	{
		EN_INFO("%s", pCallbackData->pMessage);
	}

	// Just output the error message
	return VK_FALSE;
}

b8 vulkan_instance_create(vulkan_instance_config *config, const VkAllocationCallbacks *allocator, vulkan_instance *out_instance)
{
	// Get Application data
	VkApplicationInfo app_info = {0};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = config->name;
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = config->name;
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	// Fill out instance create info struct
	VkInstanceCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;

	// Available extensions
	unsigned int extension_count = 0;
	vkEnumerateInstanceExtensionProperties(0, &extension_count, 0);
	VkExtensionProperties *available_extensions = darray_create_size(VkExtensionProperties, extension_count);
	vkEnumerateInstanceExtensionProperties(0, &extension_count, available_extensions);

	EN_INFO("Available extensions:");
	for (unsigned int i = 0; i < extension_count; i++)
	{
		EN_INFO(available_extensions[i].extensionName);
	}

	EN_DEBUG("Required extensions: ");
	// Output required extension
	u32 length = darray_length(config->extensions);
	for (unsigned int i = 0; i < length; i++)
	{
		const char *extension = config->extensions[i];
		EN_DEBUG(extension);
	}

	create_info.enabledExtensionCount = (uint32_t)darray_length(config->extensions);
	create_info.ppEnabledExtensionNames = (const char **)config->extensions;
	create_info.enabledLayerCount = 0;

// Enable validation layer support in debug mode
#ifdef _DEBUG
	// Get available validation layers
	unsigned int validation_layer_count = 0;
	vkEnumerateInstanceLayerProperties(&validation_layer_count, 0);
	VkLayerProperties *available_validation_layers = darray_create_size(VkLayerProperties, validation_layer_count);
	vkEnumerateInstanceLayerProperties(&validation_layer_count, available_validation_layers);

	// Instance VOR vkCreateInstance konfigurieren:
	b8 all_found = true;
	u64 required_count = darray_length(config->validation_layers);

	for (u32 j = 0; j < required_count; j++)
	{
		b8 found = false;
		for (u32 i = 0; i < validation_layer_count; i++)
		{
			if (string_compare(available_validation_layers[i].layerName,
							   config->validation_layers[j]))
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			all_found = false;
			break;
		}
	}

	// If all have been found add them to the vkInstanceCreateInfo
	if (all_found)
	{
		create_info.enabledLayerCount = (uint32_t)darray_length(config->validation_layers);
		create_info.ppEnabledLayerNames = config->validation_layers;
	}

	// Can only create Debug Messenger after Instance has already been created
	darray_destroy(available_validation_layers);
#endif

	// Create instance
	out_instance->allocator = allocator;
	VK_CHECK(vkCreateInstance(&create_info, out_instance->allocator, &out_instance->handle));
	darray_destroy(available_extensions);


	// Only create debug messenger after instance has been created
#ifdef _DEBUG
	create_debug_messenger(out_instance, &out_instance->debug_messenger);
#endif
	EN_DEBUG("Vulkan instance created!");
}

void vulkan_instance_destroy(vulkan_instance *instance)
{
	EN_DEBUG("Destroying Vulkan Instance.");
	if (instance->handle)
	{
		vkDestroyInstance(instance->handle, instance->allocator);
	}
}

b8 create_debug_messenger(const vulkan_instance *instance, VkDebugUtilsMessengerEXT *debug_messenger)
{
	VkDebugUtilsMessengerCreateInfoEXT create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info.messageSeverity = /*VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | */ VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info.pfnUserCallback = debug_callback;

	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance->handle, "vkCreateDebugUtilsMessengerEXT");
	VkResult result = func(instance->handle, &create_info, instance->allocator, debug_messenger);
	if (result != VK_SUCCESS)
	{
		EN_ERROR("Failed to create Debug Messenger.");
		return false;
	}

	EN_INFO("Vulkan debug messenger created.");
	return true;
}

b8 vulkan_surface_create(HWND *window_handle, HINSTANCE *windows_instance, vulkan_instance *instance, VkAllocationCallbacks *allocator, vulkan_surface *out_surface)
{
	VkWin32SurfaceCreateInfoKHR create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	create_info.hinstance = *windows_instance;
	create_info.hwnd = *window_handle;

	out_surface->instance = instance;
	out_surface->windows_instance = windows_instance;
	VkResult result = vkCreateWin32SurfaceKHR(instance->handle, &create_info, instance->allocator, &out_surface->handle);
	EN_INFO("Surface created.");
	return true;
}

void vulkan_surface_destroy(vulkan_surface *surface)
{
	EN_INFO("Destroying vulkan surface.");
	if (surface->handle)
	{
		vkDestroySurfaceKHR(surface->instance->handle,
							surface->handle,
							surface->instance->allocator);
	}
}