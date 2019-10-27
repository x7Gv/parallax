#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "lib/array.h"
#include "lib/memutil.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

typedef struct _swapchain_buffers {
	VkImage image;
	VkImageView view;
} swapchain_buffer;

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
	VkCommandBuffer cmd;

	VkPhysicalDevice physical_device;
	VkDevice device;
	VkSurfaceKHR surface;

	VkFormat img_format;
	VkFormat format;
	VkSwapchainKHR swapchain;
	VkImage swapchain_img;

	GLFWwindow *window;

	uint32_t width;
	uint32_t height;

	array queue_family_properties;
	array instance_ext_names;
	array device_ext_names;
	array swapc_buffer;

	uint32_t queue_family_count;
	uint32_t swapchain_img_count;

	unsigned int queue_family_index;
	unsigned int graphics_queue_family_index;
	unsigned int present_queue_family_index;
};

void run(struct _application *ref);

void main_loop(struct _application *ref);

void init_swapchain(struct _application *ref);

void init_window(struct _application *ref);

void init_swapchain(struct _application *ref);

void create_surface(struct _application *ref);

void create_command_pool(struct _application *ref);

void init_vk(struct _application *ref);

void init_device_ext_names(struct _application *ref);

void init_instance_ext_names(struct _application *ref);

void init_physical_device(struct _application *ref);

void init_logical_device(struct _application *ref);

void cleanup(struct _application *ref);

#endif