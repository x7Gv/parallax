#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "lib/array.h"
#include "lib/memutil.h"
#include "stdbool.h"
#include "sys/time.h"

#include <cglm/cglm.h>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#define PHYSDEV(index)	arr_get(ref->physical_devices, VkPhysicalDevice, index)

typedef struct vertex_t
{
	vec3 pos;
	vec3 color;
	vec2 tex_coord;
}
vertex_t;

typedef struct ubo_t 
{
	mat4 model;
	mat4 view;
	mat4 proj;
}
ubo_t;

static VkVertexInputBindingDescription get_binding_description();

typedef struct _queue_family_indices_t
{
	uint32_t graphics_family;
	uint32_t present_family;
}
queue_family_indices_t;

typedef struct _swapchain_supp_detail
{
	VkSurfaceCapabilitiesKHR capabilities;
	array formats;
	array present_modes;
}
swapchain_supp_detail_t;

typedef struct _application application;

struct _application
{

	/**
	 * Debug stuff with LunarG Validation Layers.
	 */

	VkDebugUtilsMessengerEXT debug_messenger;

	/**
	 * Rest of stuff.
	 */

	struct timeval start_tv;

	VkDescriptorPool descriptor_pool;
	VkDescriptorSetLayout descriptor_set_layout;
	VkPipelineLayout pipeline_layout;

	VkInstance vk_instance;
	VkCommandPool cmd_pool;

	VkDevice device;
	VkSurfaceKHR surface;

	VkQueue graphics_queue;
	VkQueue present_queue;

	VkImage depth_image;
	VkDeviceMemory depth_image_memory;
	VkImageView depth_image_view;

	VkBuffer vertex_buffer;
	VkBuffer uniform_buffer;
	VkBuffer index_buffer;

	VkSampler texture_sampler;
	VkImageView texture_image_view;
	VkImage texture_image;
	VkDeviceMemory texture_image_memory;

	VkDeviceMemory vertex_buffer_memory;
	VkDeviceMemory uniform_buffer_memory;
	VkDeviceMemory index_buffer_memory;

	VkFormat swapc_img_format;
	VkExtent2D swapc_extent;
	VkSwapchainKHR swapchain;

	VkRenderPass render_pass;
	VkPipeline graphics_pipeline;

	array img_available_semaphore;
	array render_finished_semaphore;
	array in_flight_fences;
	array imgs_in_flight;

	GLFWwindow *window;

	uint32_t width;
	uint32_t height;

	array queue_family_properties;
	array instance_ext_names;
	array device_ext_names;
	array physical_devices;

	array swapc_buffer;
	array swapc_imgs;
	array swapc_img_views;
	array swapc_framebuffers;

	array cmd_buffers;

	array uniform_buffers;
	array uniform_buffers_memory;

	array descriptor_sets;

	uint32_t queue_family_count;
	uint32_t swapchain_img_count;

	unsigned int queue_family_index;
	unsigned int graphics_queue_family_index;
	unsigned int present_queue_family_index;

	bool framebuffer_resized;
};

void create_depth_resources(struct _application *ref);

void create_texture_sampler(struct _application *ref);

void create_texture_image_view(struct _application *ref);

VkCommandBuffer begin_single_time_commands(struct _application *ref);

void end_single_time_commands(struct _application *ref, VkCommandBuffer command_buffer);

queue_family_indices_t query_queue_families(VkPhysicalDevice phys_device, VkSurfaceKHR surface);

void transition_image_layout(struct _application *ref, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);

void copy_buffer_to_image(struct _application *ref, VkBuffer buffer, VkImage image, uint32_t x, uint32_t y);

void create_image(struct _application *ref, uint32_t x, uint32_t y, VkFormat format, VkImageTiling tiling, 
	VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage *img, VkDeviceMemory *mem);

void create_buffer(struct _application *ref, VkDeviceSize size, VkBufferUsageFlags usage, 
	VkMemoryPropertyFlags props, VkBuffer *buffer, VkDeviceMemory *buffer_mem);

void copy_buffer(struct _application *ref, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);

void create_texture_image(struct _application *ref);

int check_validation_layer_support();

void update_uniform_buffer(struct _application *ref, uint32_t current_image);

void create_descriptor_set_layout(struct _application *ref);

void create_descriptor_sets(struct _application *ref);

void query_req_ext(array *arr);

void run(struct _application *ref);

void main_loop(struct _application *ref);

void draw_frame(struct _application *ref);

void create_sync_objects(struct _application *ref);

void init_window(struct _application *ref);

void init_image_views(struct _application *ref);

void create_surface(struct _application *ref);

void create_command_pool(struct _application *ref);

void init_vk(struct _application *ref);

void init_device_ext_names(struct _application *ref);

void init_instance_ext_names(struct _application *ref);

void init_physical_device(struct _application *ref);

void init_logical_device(struct _application *ref);

void create_graphics_pipeline(struct _application *ref);

void create_framebuffers(struct _application *ref);

void create_command_pool(struct _application *ref);

uint32_t find_memory_type(struct _application *ref, uint32_t type_filter, VkMemoryPropertyFlags properties);

void create_index_buffer(struct _application *ref);

void create_vertex_buffer(struct _application *ref);

void create_uniform_buffers(struct _application *ref);

void create_command_buffers(struct _application *ref);

void cleanup(struct _application *ref);

void create_renderpass(struct _application *ref);

void create_descriptor_pool(struct _application *ref);

#endif
