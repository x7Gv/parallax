#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "application.h"

#include "stdio.h"
#include "stdlib.h"

void run(struct _application *ref)
{
	init_window(ref);
	init_vk(ref);

	main_loop(ref);

	cleanup(ref);
}

void main_loop(struct _application *ref)
{
	while (!glfwWindowShouldClose(ref->window))
	{
		glfwPollEvents();
	}
}

void init_window(struct _application *ref)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	ref->window = glfwCreateWindow(800, 600, "parallax", NULL, NULL);
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

	pick_physical_device(ref);
}

void pick_physical_device(struct _application *ref)
{
	uint32_t dev_count = 0;
	vkEnumeratePhysicalDevices(ref->vk_instance, &dev_count, NULL);

	if (dev_count == 0) {
		fprintf(stderr, "ERR: failed to pick a physical device\n // Assertion: `vkPhysicalDeviceCount > 0`");
		exit(EXIT_FAILURE);
	}

	arr_init(devices);
	array_resize(&devices, dev_count);

	int res = vkEnumeratePhysicalDevices(ref->vk_instance, &dev_count, (VkPhysicalDevice*) array_data(&devices));
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to pick a physical device\n // Assertion: `vkEnumeratePhysicalDevices != VK_SUCCESS`");
		exit(EXIT_FAILURE);
	}

	ref->physical_device = arr_get(devices, VkPhysicalDevice, 0);
}

void cleanup(struct _application *ref)
{
	vkDestroyInstance(ref->vk_instance, NULL);

	glfwDestroyWindow(ref->window);
	glfwTerminate();
} 
