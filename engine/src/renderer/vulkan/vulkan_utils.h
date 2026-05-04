#pragma once

#include <vulkan/vulkan.h>
#include "core/logger.h"

#define VK_CHECK(expr) if(expr != VK_SUCCESS) { EN_ERROR("Error executing %s", #expr); }


const char* vulkan_result_string(VkResult result, b8 getExtended);