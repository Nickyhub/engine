#include "vulkan_device.h"
#include "vulkan_utils.h"
#include "core/estring.h"
#include "core/logger.h"
#include "core/platform.h"
#include "containers/darray.h"

b8 query_swapchain_support(vulkan_device *device, VkPhysicalDevice physical_device);
VkPhysicalDevice select_physical_device(vulkan_device *device);

b8 physical_device_meets_requirements(
	VkPhysicalDevice physical_device,
	vulkan_device *device,
	vulkan_physical_device_requirements *requirements);

b8 vulkan_device_create(
	vulkan_surface *surface,
	vulkan_instance *instance,
	b8 enable_sampler_anisotropy,
	b8 enable_fill_mode_non_solid,
	vulkan_device *out_device)
{
	out_device->instance = instance;
	out_device->surface = surface;
	out_device->support_info.formats = darray_create(VkSurfaceFormatKHR);
	out_device->support_info.present_modes = darray_create(VkPresentModeKHR);

	out_device->physical_device = select_physical_device(out_device);
	if (!out_device->physical_device)
	{
		EN_ERROR("No physical device present.");
		return false;
	}

	// Logical device
	// Get unique indices in case queue families share indices
	u32 family_indices[4];
	family_indices[0] = out_device->graphics_queue_family_index;
	family_indices[1] = out_device->present_queue_family_index;
	family_indices[2] = out_device->transfer_queue_family_index;
	family_indices[3] = out_device->compute_queue_family_index;

	u32 *unique_family_indices = darray_create(u32);
	// Put at least the first index in the list because at this point it will be unique
	darray_push_back(unique_family_indices, family_indices[0]);
	// Filter out unique indices
	for (unsigned int i = 0; i < 4; i++)
	{
		b8 is_unique = false;
		for (unsigned int j = 0; j < darray_length(unique_family_indices); j++)
		{
			u32 family_index = family_indices[i];
			u32 unique_family_index = unique_family_indices[j];
			if (family_indices[i] != unique_family_indices[j])
			{
				is_unique = true;
				break;
			}
		}
		if (is_unique)
		{
			u32 family_index = family_indices[i];
			darray_push_back(unique_family_indices, family_index);
		}
	}

	// Create and fill queue create infos
	VkDeviceQueueCreateInfo *queue_create_infos = darray_create(VkDeviceQueueCreateInfo);
	u64 length = darray_length(unique_family_indices);
	float queuePriority = 1.0f;
	for (unsigned int i = 0; i < length; i++)
	{
		u32 unique_family_index = unique_family_indices[i];
		if (unique_family_index != -1)
		{
			VkDeviceQueueCreateInfo queue_create_info;
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = unique_family_index;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &queuePriority;
			darray_push_back(queue_create_infos, queue_create_info);
		}
		else
		{
			EN_WARN("Graphics queue family index does not exist. No Graphics Queue created.");
		}
	}

	// TODO come back and specify the device features we want to use
	VkPhysicalDeviceFeatures device_features;
	device_features.samplerAnisotropy = enable_sampler_anisotropy ? VK_TRUE : VK_FALSE; // Request anistrophy
	device_features.fillModeNonSolid = enable_fill_mode_non_solid ? VK_TRUE : VK_FALSE;

	// Creating the logical device
	VkDeviceCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pQueueCreateInfos = queue_create_infos;
	create_info.queueCreateInfoCount = (uint32_t)darray_length(queue_create_infos);
	create_info.pEnabledFeatures = &device_features;

	const char *swapchain_extension_name = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

	const char **extension_names = &swapchain_extension_name;
	create_info.enabledExtensionCount = 1;
	create_info.ppEnabledExtensionNames = extension_names;

	create_info.enabledLayerCount = 0;
	create_info.ppEnabledLayerNames = 0;

	VK_CHECK(vkCreateDevice(out_device->physical_device, &create_info, instance->allocator, &out_device->handle));

	// Get the queue handles after logical device creation
	VkDevice d = out_device->handle;
	vkGetDeviceQueue(d, out_device->graphics_queue_family_index, 0, &out_device->graphics_queue);
	vkGetDeviceQueue(d, out_device->present_queue_family_index, 0, &out_device->present_queue);
	vkGetDeviceQueue(d, out_device->transfer_queue_family_index, 0, &out_device->transfer_queue);
	vkGetDeviceQueue(d, out_device->compute_queue_family_index, 0, &out_device->compute_queue);

	// Create command pool for graphics queue
	VkCommandPoolCreateInfo pool_info;
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = out_device->graphics_queue_family_index;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECK(vkCreateCommandPool(out_device->handle, &pool_info, instance->allocator, &out_device->command_pool));
	darray_destroy(unique_family_indices);
	darray_destroy(queue_create_infos);

	EN_DEBUG("Command pool created for graphics queue.");
	EN_DEBUG("Logical device created.");
	return true;
}

void vulkan_device_destroy(vulkan_device *device)
{
	darray_destroy(device->support_info.formats);
	darray_destroy(device->support_info.present_modes);
	vkDestroyCommandPool(device->handle, device->command_pool, device->instance->allocator);
	vkDestroyDevice(device->handle, device->instance->allocator);
	EN_DEBUG("vulkan_device destroyed.");
}

b8 vulkan_device_find_depth_format(vulkan_device *device)
{
	// Format candidates
	VkFormat candidates[3];
	candidates[0] = (VK_FORMAT_D32_SFLOAT);
	candidates[1] = (VK_FORMAT_D32_SFLOAT_S8_UINT);
	candidates[2] = (VK_FORMAT_D24_UNORM_S8_UINT);

	unsigned int flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
	for (int i = 0; i < 3; ++i)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(device->physical_device, candidates[i], &properties);

		if ((properties.linearTilingFeatures & flags) == flags)
		{
			device->depth_format = candidates[i];
			return true;
		}
		else if ((properties.optimalTilingFeatures & flags) == flags)
		{
			device->depth_format = candidates[i];
			return true;
		}
	}
	return false;
}

VkPhysicalDevice select_physical_device(vulkan_device *device)
{
	unsigned int device_count = 0;
	vkEnumeratePhysicalDevices(device->instance->handle,
							   &device_count,
							   0);

	VkPhysicalDevice *physical_devices = darray_create_size(VkPhysicalDevice, device_count);
	VkPhysicalDeviceProperties properties;

	// Fill out requirements for the Device and choose it based on them
	vulkan_physical_device_requirements requirements;
	requirements.required_extensions = darray_create(const char *);
	requirements.compute_queue = true;
	requirements.graphics_queue = true;
	requirements.present_queue = true;
	requirements.discrete_gpu = true;
	requirements.transfer_queue = true;
	requirements.sampler_anisotropy = true;

	darray_push_back(requirements.required_extensions, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	if (device_count == 0)
	{
		EN_FATAL("Vulkan can not find a physical device. Cannot continue application.");
		return 0;
	}
	else if (device_count == 1)
	{
		vkEnumeratePhysicalDevices(device->instance->handle,
								   &device_count,
								   physical_devices);
		vkGetPhysicalDeviceProperties(physical_devices[0], &properties);
		if (!physical_device_meets_requirements(physical_devices[0], device, &requirements))
		{
			EN_FATAL("There is no device that meets the physical requirements. Cannot continue.");
			return 0;
		}
		EN_INFO("Physical device chosen: %s", properties.deviceName);
		return physical_devices[0];
	}

	// Multiple devices are present. Choose the most suitable
	u32 *physical_device_scores = darray_create(u32);

	// Give every device a score
	for (unsigned int i = 0; i < device_count; i++)
	{
		vkGetPhysicalDeviceProperties(physical_devices[i], &properties);
		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(physical_devices[i], &features);
		device->physical_device = physical_devices[i];
		if (!physical_device_meets_requirements(physical_devices[i], device, &requirements))
		{
			u32 score_add = 100;
			physical_device_scores[i] += score_add;
			EN_INFO("Physical device %s does not meet the requiremtns. Skipping...", properties.deviceName);
			break;
		}

		u32 score = physical_device_scores[i];
		score += properties.limits.maxImageDimension2D;
		if (!features.geometryShader)
		{
			EN_WARN("Geometry shader not present on device: %s", properties.deviceName);
			score = physical_device_scores[i];
			score -= 10;
		}
	}

	// Choose the device with the highest score
	u32 score = 0;
	unsigned int device_index = 0;
	for (unsigned int i = 0; i < device_count; i++)
	{
		u32 device_score = physical_device_scores[i];
		if (device_score >= score)
		{
			score = device_score;
			device_index = i;
		}
	}
	if (physical_devices[device_index])
	{
		return physical_devices[device_index];
	}
	darray_destroy(requirements.required_extensions);
	darray_destroy(physical_device_scores);
	darray_destroy(physical_devices);
	EN_FATAL("No physical device was picked. Cannot continue.");
	return 0;
}

b8 physical_device_meets_requirements(
	VkPhysicalDevice physical_device,
	vulkan_device *device,
	vulkan_physical_device_requirements *requirements)
{
	// Check if the device is a discrete GPU
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(physical_device, &properties);

	if ((properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) && requirements->discrete_gpu)
	{
		EN_INFO("Device %s is not a discrete GPU. Skipping.", properties.deviceName);
		return false;
	}

	unsigned int queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, 0);

	VkQueueFamilyProperties *queue_families = darray_create_size(VkQueueFamilyProperties, queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);

	u32 min_transfer_score = 255;

	// Local indices to check them later
	unsigned int graphics_index = -1, present_index = -1, compute_index = -1, transfer_index = -1;

	for (unsigned int i = 0; i < queue_family_count; i++)
	{
		// Graphics queue?
		u32 current_transfer_score = 0;
		u32 flags = queue_families[i].queueFlags;
		if (flags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphics_index = i;
			VkBool32 supports_present = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(physical_device,
												 i,
												 device->surface->handle,
												 &supports_present);
			if (supports_present)
			{
				present_index = i;
				++current_transfer_score;
			}
			++current_transfer_score;
		}
		// Compute queue?
		if (flags & VK_QUEUE_COMPUTE_BIT)
		{
			compute_index = i;
			++current_transfer_score;
		}
		// Transfer queue?
		if (flags & VK_QUEUE_TRANSFER_BIT)
		{
			// Prefer dedicated transfer queue
			// Take the index if it is the current lowest. This increases the
			// liklihood that it is a dedicated transfer queue.
			if (current_transfer_score <= min_transfer_score)
			{
				min_transfer_score = current_transfer_score;
				transfer_index = i;
			}
		}
	}

	darray_destroy(queue_families);
	// It is possible that no present queue has been found with the same graphics queue index
	// Iterate again and choose the first present queue found
	if (present_index == -1)
	{
		for (unsigned int i = 0; i < queue_family_count; i++)
		{
			VkBool32 supports_present = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(physical_device,
												 i,
												 device->surface->handle,
												 &supports_present);
			if (supports_present)
			{
				present_index = i;

				if (present_index != graphics_index)
				{
					EN_WARN("Different queue index used for present vs graphics: %u.", i);
				}
				break;
			}
		}
	}

	if ((!requirements->graphics_queue || requirements->graphics_queue && graphics_index != -1) &&
		(!requirements->present_queue || requirements->present_queue && present_index != -1) &&
		(!requirements->compute_queue || requirements->compute_queue && compute_index != -1) &&
		(!requirements->transfer_queue || requirements->transfer_queue && transfer_index != -1))
	{

		EN_INFO("Device meets queue requirements. ");

		// Print info about the device
		EN_INFO("Graphics queue index: %u.", graphics_index);
		EN_INFO("Present queue index: %u.", present_index);
		EN_INFO("Compute queue index: %u.", compute_index);
		EN_INFO("Transfer queue index: %u.", transfer_index);

		// Copy over the indices from local variables to the VulkanDevice
		device->graphics_queue_family_index = graphics_index;
		device->present_queue_family_index = present_index;
		device->compute_queue_family_index = compute_index;
		device->transfer_queue_family_index = transfer_index;

		// Fill the struct with physical device features, properties and memory infos
		vkGetPhysicalDeviceFeatures(physical_device, &device->features);
		vkGetPhysicalDeviceProperties(physical_device, &device->properties);
		vkGetPhysicalDeviceMemoryProperties(physical_device, &device->mem_props);

		// Output some more info
		;
		EN_INFO(
			"GPU Driver version: %d.%d.%d",
			VK_VERSION_MAJOR(device->properties.driverVersion),
			VK_VERSION_MINOR(device->properties.driverVersion),
			VK_VERSION_PATCH(device->properties.driverVersion));

		// Vulkan API version.
		EN_INFO(
			"Vulkan API version: %d.%d.%d",
			VK_VERSION_MAJOR(device->properties.apiVersion),
			VK_VERSION_MINOR(device->properties.apiVersion),
			VK_VERSION_PATCH(device->properties.apiVersion));

		// Memory info
		for (u32 j = 0; j < device->mem_props.memoryHeapCount; ++j)
		{
			float memory_size_gib = (((float)device->mem_props.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);
			if (device->mem_props.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
			{
				EN_INFO("Local GPU memory: %.2f GiB", memory_size_gib);
			}
			else
			{
				EN_INFO("Shared System memory: %.2f GiB", memory_size_gib);
			}
		}

		// Check if extensions are available
		u32 available_extension_count = 0;
		vkEnumerateDeviceExtensionProperties(physical_device, 0, &available_extension_count, 0);
		VkExtensionProperties *available_device_extensions = darray_create_size(VkExtensionProperties, available_extension_count);
		vkEnumerateDeviceExtensionProperties(physical_device, 0, &available_extension_count, available_device_extensions);

		for (u32 i = 0; i < darray_length(requirements->required_extensions); i++)
		{
			b8 found = false;
			for (u32 j = 0; j < available_extension_count; j++)
			{
				VkExtensionProperties *extension_name = &available_device_extensions[j];
				const char *required_extension = requirements->required_extensions[i];
				if (string_compare(extension_name->extensionName, required_extension))
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				i8 *extension = (i8 *)requirements->required_extensions[i];
				EN_INFO("Required device extension %s not found. Skipping.", extension);
				darray_destroy(available_device_extensions);
				return false;
			}
		}

		darray_destroy(available_device_extensions);

		if (!vulkan_device_query_swapchain_support(device, physical_device))
		{
			EN_ERROR("Device does not meet the swapchain support requirements. Skipping.");
			return false;
		}
		// Check sampler anisotropy
		if (requirements->sampler_anisotropy && !device->features.samplerAnisotropy)
		{
			EN_INFO("Device does not support samplerAnisotropy, skipping.");
			return false;
		}
		return true;
	}
	else
	{
		EN_ERROR("Device does not meet queue requirements. Skipping.");
		return false;
	}
	return false;
}

b8 vulkan_device_query_swapchain_support(vulkan_device *device, VkPhysicalDevice physical_device)
{
	// Create information structs to be filled
	VkSurfaceCapabilitiesKHR capabilities = {0};
	// std::vector<VkSurfaceFormatKHR> formats;
	// std::vector<VkPresentModeKHR> presentModes;

	// Query surface capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, device->surface->handle, &capabilities);

	// Query formats
	unsigned int format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, device->surface->handle, &format_count, 0);
	// Check if formats are available
	if (format_count != 0)
	{
		// darray_resize(device->support_info.formats, format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			physical_device,
			device->surface->handle,
			&format_count,
			device->support_info.formats);
	}
	else
	{
		EN_ERROR("Physical device has no surface formats available. Skipping.");
		return false;
	}

	// Query present modes
	unsigned int present_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, device->surface->handle, &present_mode_count, 0);
	// Check if present modes are available
	if (present_mode_count != 0)
	{
		// darray_resize(device->support_info.present_modes, present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			physical_device,
			device->surface->handle,
			&present_mode_count,
			device->support_info.present_modes);
	}
	else
	{
		EN_ERROR("Physical device has no surface present modes available. Skipping.");
		return false;
	}
	device->support_info.format_count = format_count;
	device->support_info.present_mode_count = present_mode_count;
	// Set capabilities
	device->support_info.capabilities = capabilities;

	EN_INFO("Vulkan swapchain support is sufficient and data has been copied to vulkan data.");
	return true;
}