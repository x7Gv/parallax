#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "lib/array.h"

typedef struct _application application;

struct _application
{
	GLFWwindow *window;

	VkInstance vk_instance;
	VkPhysicalDevice physical_device;

	array queue_family_properties;
	uint32_t queue_family_count;
};

void run(struct _application *ref);

void main_loop(struct _application *ref);

void init_window(struct _application *ref);

void init_vk(struct _application *ref);

void pick_physical_device(struct _application *ref);

void cleanup(struct _application *ref);

#endif