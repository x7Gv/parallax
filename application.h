#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "lib/array.h"
#include "lib/memutil.h"
#include "stdbool.h"

#include <cglm/cglm.h>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#define PHYSDEV(index)	arr_get(ref->physical_devices, VkPhysicalDevice, index)

typedef struct vertex_t
{
	vec2 pos;
	vec3 color;
}
vertex_t;

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

	VkInstance vk_instance;
	VkCommandPool cmd_pool;

	VkDevice device;
	VkSurfaceKHR surface;

	VkQueue graphics_queue;
	VkQueue present_queue;

	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_memory;
	VkBuffer uniform_buffer;
	VkDeviceMemory uniform_buffer_memory;

	VkFormat swapc_img_format;
	VkExtent2D swapc_extent;
	VkSwapchainKHR swapchain;

	VkRenderPass render_pass;
	VkPipelineLayout pipeline_layout;
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

	uint32_t queue_family_count;
	uint32_t swapchain_img_count;

	unsigned int queue_family_index;
	unsigned int graphics_queue_family_index;
	unsigned int present_queue_family_index;

	bool framebuffer_resized;
};

queue_family_indices_t query_queue_families(VkPhysicalDevice phys_device, VkSurfaceKHR surface);

int check_validation_layer_support();

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

void create_vertex_buffer(struct _application *ref);

void create_command_buffers(struct _application *ref);

void cleanup(struct _application *ref);

void create_renderpass(struct _application *ref);

#endif