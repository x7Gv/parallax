#include "application.h"

#include "vulkan/vulkan.h"

#include "stdio.h"
#include "stdlib.h"

void run(struct _application *ref)
{
	init_vk(ref);
	cleanup(ref);
}

void main_loop(struct _application *ref)
{
	// to be implemented.
}

void init_vk(struct _application *ref)
{
	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = NULL;	
	app_info.pApplicationName = "parallax";
	app_info.applicationVersion = VK_MAKE_VERSION(0, 9, 0);
	app_info.pEngineName = "parallax-eng";
	app_info.engineVersion = VK_MAKE_VERSION(0, 9, 0);
	app_info.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo instance_ci = {};
	instance_ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_ci.pNext = NULL;
	instance_ci.flags = 0;
	instance_ci.pApplicationInfo = &app_info;

	int res = vkCreateInstance(&instance_ci, NULL, &ref->vk_instance);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: Failed to instantiate Vulkan\n// Assertion: `vkCreateInstance() == VK_SUCCES`\n");
		exit(EXIT_FAILURE);
	}
}

void cleanup(struct _application *ref)
{
	vkDestroyInstance(ref->vk_instance, NULL);
} 
