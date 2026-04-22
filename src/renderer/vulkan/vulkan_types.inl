#pragma once

#include <windows.h>
#include <vulkan/vulkan.h>

#include "containers/darray.h"
#include "renderer/renderer_types.inl"

typedef struct vulkan_instance
{
	const VkAllocationCallbacks *allocator;
#ifdef _DEBUG
	VkDebugUtilsMessengerEXT debug_messenger;
#endif
	VkInstance handle;
} vulkan_instance;

typedef struct vulkan_surface
{
	VkSurfaceKHR handle;
	const HINSTANCE *windows_instance;
	vulkan_instance *instance;
} vulkan_surface;

typedef struct vulkan_device_swapchain_support_info
{
	unsigned int format_count;
	// VkSurfaceFormatKHR
	VkSurfaceFormatKHR *formats; // darray

	unsigned int present_mode_count;
	VkPresentModeKHR *present_modes; // darray

	VkSurfaceCapabilitiesKHR capabilities;
} vulkan_device_swapchain_support_info;

typedef struct vulkan_device
{
	const vulkan_instance *instance;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceMemoryProperties mem_props;

	VkQueue graphics_queue;
	VkQueue compute_queue;
	VkQueue present_queue;
	VkQueue transfer_queue;

	const vulkan_surface *surface;
	vulkan_device_swapchain_support_info support_info;
	u32 graphics_queue_family_index;
	u32 compute_queue_family_index;
	u32 present_queue_family_index;
	u32 transfer_queue_family_index;

	VkPhysicalDevice physical_device;
	VkDevice handle;
	VkFormat depth_format;

	VkCommandPool command_pool;
} vulkan_device;

typedef struct vulkan_buffer
{
	VkBuffer handle;
	VkDeviceMemory memory;
	VkBufferUsageFlagBits usage;
	b8 is_locked;
	i32 memory_index;
	u32 memory_property_flags;
	const vulkan_device *device;
	const VkAllocationCallbacks *allocator;
	VkDeviceSize size;
} vulkan_buffer;

typedef enum vulkan_command_buffer_state
{
	COMMAND_BUFFER_CREATED,
	COMMAND_BUFFER_RECORDING,
	COMMAND_BUFFER_RECORDING_ENDED,
	COMMAND_BUFFER_DEALLOCATED,
} vulkan_command_buffer_state;

typedef struct vulkan_command_buffer
{
	vulkan_command_buffer_state state;
	VkCommandBuffer handle;
	const vulkan_device *device;
	VkCommandPool command_pool;
} vulkan_command_buffer;

typedef struct vulkan_renderpass
{
	VkRenderPass handle;
	const vulkan_device *device;
	const VkAllocationCallbacks *allocator;
} vulkan_renderpass;

typedef struct vulkan_pipeline
{
	VkDescriptorSetLayout descriptor_set_layout;
	VkPipelineLayout layout;
	VkDescriptorSet *descriptor_sets; // darray
	VkDescriptorPool descriptor_pool;

	const vulkan_device *device;
	const VkAllocationCallbacks *allocator;

	u32 frames_in_flight;
	VkPipeline handle;
} vulkan_pipeline;

#define OBJECT_SHADER_STAGE_COUNT 2
typedef struct vulkan_shader_stage
{
	VkShaderModuleCreateInfo create_info;
	VkShaderModule handle;
	VkPipelineShaderStageCreateInfo shader_stage_create_info;
} vulkan_shader_stage;

// These are multiple shaders basically
// Each stage is a shader like fragment and vertex for ex.
typedef struct vulkan_object_shader
{
	// currently: vertex, fragment
	vulkan_shader_stage stages[OBJECT_SHADER_STAGE_COUNT];
	vulkan_pipeline pipeline;
	const vulkan_device *device;
	const VkAllocationCallbacks *allocator;
} vulkan_object_shader;

typedef struct vulkan_physical_device_requirements
{
	b8 graphics_queue;
	b8 present_queue;
	b8 transfer_queue;
	b8 compute_queue;
	const char **required_extensions; // darray

	b8 sampler_anisotropy;
	b8 discrete_gpu;
} vulkan_physical_device_requirements;

typedef struct vulkan_framebuffer
{
	VkFramebuffer handle;
	const VkAllocationCallbacks *allocator;
	const vulkan_device *device;
} vulkan_framebuffer;

typedef struct vulkan_image
{
	VkImage handle;
	const vulkan_device *device;
	const VkAllocationCallbacks *allocator;
	VkDeviceMemory memory;

	u32 width;
	u32 height;
	u16 channels;
	VkSampler sampler;
	VkImageView view;
} vulkan_image;

typedef struct vulkan_semaphore
{
	VkSemaphore handle;
	const vulkan_device *device;
	const VkAllocationCallbacks *allocator;
} vulkan_semaphore;

typedef struct vulkan_fence
{
	VkFence handle;
	const vulkan_device *device;
	const VkAllocationCallbacks *allocator;
} vulkan_fence;

typedef struct vulkan_swapchain
{
	u16 width;
	u16 height;
	u16 image_count;
	u32 current_swapchain_image_index;

	u32 max_frames_in_flight;

	const VkAllocationCallbacks *allocator;
	vulkan_device *device;
	VkExtent2D extent;
	VkSurfaceFormatKHR surface_format;

	VkImageView *image_views;		  // darray
	VkImage *images;				  // darray
	vulkan_framebuffer *framebuffers; // darray

	vulkan_image depth_image;

	VkSwapchainKHR handle;
	VkPresentModeKHR present_mode;

	u32 image_view_count;
} vulkan_swapchain;

typedef struct vulkan_context
{
	u16 current_frame_in_flight_index;

	vulkan_instance instance;
	vulkan_surface surface;
	vulkan_device device;
	vulkan_swapchain swapchain;
	b8 recreating_swapchain;

	vulkan_semaphore *image_available_semaphores; // darray
	vulkan_semaphore *render_finished_semaphores; // darray
	vulkan_fence *in_flight_fences;				  // darray

	u32 framebuffer_height;
	u32 framebuffer_width;
	u32 framebuffer_generation;
	u32 framebuffer_last_generation;

	vulkan_image image;

	vulkan_buffer object_vertex_buffer;
	vulkan_buffer object_index_buffer;

	vulkan_renderpass renderpass;
	vulkan_object_shader object_shader;

	// Maintain offset into the buffers
	u64 geometry_vertex_offset;
	u64 geometry_index_offset;

	// vulkan_command_buffer
	vulkan_command_buffer *command_buffers; // darray

	VkAllocationCallbacks *allocator;
} vulkan_context;