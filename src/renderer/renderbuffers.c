#include "renderbuffers.h"
#include "renderer/vulkan/vulkan_buffer.h"
#include "core/logger.h"

b8 vertex_buffer_create(
    vertex_3d *vertices,
    const vulkan_device *device,
    const VkAllocationCallbacks *allocator,
    vertex_buffer *out_vertex_buffer)
{
    out_vertex_buffer->device = device;

    if (darray_length(vertices) == 0)
    {
        EN_WARN("CreateVertexBuffer was called with an empty set of vertices. Nothing happens.");
        return false;
    }

    // First calculate and set the buffer size.
    u32 buffer_size = darray_length(vertices) * sizeof(vertex_3d);

    vulkan_buffer staging_buffer = {0};
    vulkan_buffer_create(device,
                         buffer_size,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         out_vertex_buffer->allocator,
                         &staging_buffer);

    // Map the memory of the geometry data to the staging buffers memory
    void *data = 0;
    vkMapMemory(device->handle,
                staging_buffer.memory,
                0,
                buffer_size,
                0,
                &data);
    memcpy(data, vertices, darray_length(vertices) * sizeof(vertex_3d));
    vkUnmapMemory(device->handle, staging_buffer.memory);

    vulkan_buffer_create(device,
                         buffer_size,
                         (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT),
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         allocator,
                         &out_vertex_buffer->internal_buffer);

    if (!vulkan_buffer_copy_buffer(&staging_buffer,
                                   &out_vertex_buffer->internal_buffer,
                                   buffer_size,
                                   device->graphics_queue))
    {
        EN_ERROR("Failed to copy staging buffer to actual vulkan buffer.");
    }
    // Destroy the temporary staging buffer
    vulkan_buffer_destroy(&staging_buffer);
    EN_INFO("Vertex buffer created.");
    return true;
}

void vertex_buffer_destroy(vertex_buffer *vertex_buffer)
{
    vertex_buffer->allocator = 0;
    vertex_buffer->device = 0;
    vulkan_buffer_destroy(&vertex_buffer->internal_buffer);
}

b8 index_buffer_create(u32 *indices,
                       const vulkan_device *device,
                       const VkAllocationCallbacks *allocator,
                       index_buffer *out_index_buffer)
{
    out_index_buffer->device = device;
    out_index_buffer->allocator = allocator;
    if (darray_length(indices) == 0)
    {
        EN_WARN("CreateIndexBuffer was called with an empty set of vertices. Nothing happens.");
    }
    // Calculate and set index buffer size
    size_t buffer_size = sizeof(unsigned int) * darray_length(indices);


    vulkan_buffer staging_buffer = {0};
    vulkan_buffer_create(device,
                         buffer_size,
                         (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         allocator,
                         &staging_buffer);

    void *data;
    vkMapMemory(device->handle,
                staging_buffer.memory,
                0,
                buffer_size,
                0,
                &data);
    memcpy(data, indices, buffer_size);
    vkUnmapMemory(device->handle, staging_buffer.memory);

    vulkan_buffer_create(device,
                         buffer_size,
                        (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        allocator,
                        &out_index_buffer->internal_buffer);

    vulkan_buffer_copy_buffer(&staging_buffer, &out_index_buffer->internal_buffer, buffer_size, device->graphics_queue);
    // Clean up staging buffer(destructor will be called since its stack allocated)
    EN_INFO("Index buffer created.");
}

void index_buffer_destroy(index_buffer *index_buffer) {
    index_buffer->allocator = 0;
    index_buffer->device = 0;
    vulkan_buffer_destroy(&index_buffer->internal_buffer);
    EN_DEBUG("Index buffer destroyed.");
}

/*std::vector<Vertex> *VertexBuffer::generatePlaneData(unsigned int width, unsigned int height, float fieldWidth, float fieldHeight)
{
    std::vector<Vertex> *vertices = new std::vector<Vertex>();
    /*for (unsigned int i = 0; i < height; i++) {
        for (unsigned int j = 0; j < width; j++) {
            glm::vec3 v = { i* fieldWidth, j* fieldHeight, Random::GetNormalizedFloat()};
            glm::vec4 c = { Random::GetNormalizedFloat(), Random::GetNormalizedFloat() + 4, Random::GetNormalizedFloat(), 1.0f};
            vertices->push_back({ v,c });
        }
    }
    vertices->push_back({{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}});
    vertices->push_back({{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}});
    vertices->push_back({{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}});
    vertices->push_back({{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}});

    return vertices;
}*/


/*std::vector<unsigned int> *IndexBuffer::generateExampleIndices(int width, int height)
{
    std::vector<unsigned int> *indices = new std::vector<unsigned int>();

    // for (int i = 0; i < height - 1; i++) {
    //	for (int j = 0; j < width - 1; j++) {
    //		// First triangle
    //		indices->push_back(i * width + j);					// Top left
    //		indices->push_back((i * width + j) + 1);			// Top right
    //		indices->push_back((i * width) + width + j);		// Bottom left

    //		// Second Triangle
    //		indices->push_back((i * width) + width + j);		// Bottom left
    //		indices->push_back(((i * width) + width) + j + 1);	// Bottom right
    //		indices->push_back((i * width + j) + 1);			// Top right
    //	}
    //}

    indices->push_back(0);
    indices->push_back(1);
    indices->push_back(2);
    indices->push_back(2);
    indices->push_back(3);
    indices->push_back(0);
    return indices;
}
*/
