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

void create_surface(struct _application *ref)
{
	int res = glfwCreateWindowSurface(ref->vk_instance, ref->window, NULL, &ref->surface);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to create window surface via GLFW \n // Assertion: `glfwCreateWindowSurface() != VK_SUCCESS`");
		exit(EXIT_FAILURE);
	}
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
	instance_ci.pApplicationInfo = &app_info;

	uint32_t glfw_extension_count = 0;
	const char** glfw_extensions; 

	glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

	instance_ci.enabledExtensionCount = glfw_extension_count;
	instance_ci.ppEnabledExtensionNames = glfw_extensions;
	instance_ci.enabledLayerCount = 0;

	int res = vkCreateInstance(&instance_ci, NULL, &ref->vk_instance);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: Failed to instantiate Vulkan\n// Assertion: `vkCreateInstance() == VK_SUCCES`\n");
		exit(EXIT_FAILURE);
	}

	uint32_t ext_count = 0;
	vkEnumerateInstanceExtensionProperties(NULL, &ext_count, NULL);
	arr_init(extensions);
	array_resize(&extensions, ext_count);

	VkExtensionProperties tmp_props[32];
	vkEnumerateInstanceExtensionProperties(NULL, &ext_count, tmp_props);

	create_surface(ref);

	pick_physical_device(ref);
	pick_logical_device(ref);
}

void pick_physical_device(struct _application *ref)
{
	ref->physical_device = VK_NULL_HANDLE;

	uint32_t dev_count = 0;
	vkEnumeratePhysicalDevices(ref->vk_instance, &dev_count, NULL);

	if (dev_count == 0) {
		fprintf(stderr, "ERR: failed to pick a physical device\n // Assertion: `vkPhysicalDeviceCount > 0`");
		exit(EXIT_FAILURE);
	}
	
	array phys_devices;
	VkPhysicalDevice tmp_array[16];

	array_init(&phys_devices);
	array_resize(&phys_devices, dev_count);

	int res = vkEnumeratePhysicalDevices(ref->vk_instance, &dev_count, tmp_array);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to pick a physical device\n // Assertion: `vkEnumeratePhysicalDevices != VK_SUCCESS`");
		exit(EXIT_FAILURE);
	}
	
	arr_append(phys_devices, tmp_array[0]);

	VkPhysicalDevice *p_phys_tmp;
	VkPhysicalDevice phys_tmp = array_get(&phys_devices, 0);
	VAL_TO_HEAP(p_phys_tmp, VkPhysicalDevice, phys_tmp);

	ref->physical_device = *((VkPhysicalDevice *) p_phys_tmp);
}

void pick_logical_device(struct _application *ref)
{
	VkDeviceQueueCreateInfo queue_info = {};

	VkQueueFamilyProperties properties_tmp[256];

	uint32_t queue_family_count;
	vkGetPhysicalDeviceQueueFamilyProperties(ref->physical_device, &ref->queue_family_count, NULL);

	printf(">>> %d\n", ref->queue_family_count);

	if (ref->queue_family_count == 0) {
		fprintf(stderr, "ERR: failed to pick a logical device\n // Assertion: `vkGetPhysicalDeviceQueueFamilyProperties > 0`");
		exit(EXIT_FAILURE);
	}

	array_init(&ref->queue_family_properties);
	array_resize(&ref->queue_family_properties, ref->queue_family_count);

	VkQueueFamilyProperties *properties = (VkQueueFamilyProperties *) array_data(&ref->queue_family_properties);

	VkQueueFamilyProperties q_props[ref->queue_family_count];
	vkGetPhysicalDeviceQueueFamilyProperties(ref->physical_device, &ref->queue_family_count, q_props);

	for (int i = 0; i < ref->queue_family_count; i++) {
		VkQueueFamilyProperties *p_qprops_tmp;
		VkQueueFamilyProperties qprops_tmp = properties_tmp[i];
		VAL_TO_HEAP(p_qprops_tmp, VkQueueFamilyProperties, qprops_tmp);

		array_append(&ref->queue_family_properties, ((VkQueueFamilyProperties *) p_qprops_tmp));
	}

	int found = 0;
	for (uint32_t i = 0; i < ref->queue_family_count; i++) {

		VkQueueFamilyProperties prop = *((VkQueueFamilyProperties *) array_get(&ref->queue_family_properties, i));

		if (prop.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			queue_info.queueFamilyIndex = i;
			found  = 1;
			break;
		}
	}

	float queue_priorities[1] = {0.0};
	queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.pNext = NULL;
	queue_info.queueCount = 1;
	queue_info.pQueuePriorities = queue_priorities;

	VkDeviceCreateInfo device_info = {};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pNext = NULL;
	device_info.queueCreateInfoCount = 1;
	device_info.pQueueCreateInfos = &queue_info;
	device_info.enabledExtensionCount = 0;
	device_info.ppEnabledExtensionNames = NULL;
	device_info.ppEnabledLayerNames = NULL;
	device_info.pEnabledFeatures = NULL;

	VkResult res = vkCreateDevice(ref->physical_device, &device_info, NULL, &ref->device);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to pick a logical device\n // Assertion: `vkCreateDevice != VK_SUCCESS`");
		exit(EXIT_FAILURE);
	}
}

void create_command_pool(struct _application *ref)
{
	VkCommandPoolCreateInfo cmd_pool_info = {};
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.pNext = NULL;
	cmd_pool_info.queueFamilyIndex = ref->queue_family_index;
	cmd_pool_info.flags = 0;

	VkResult res = vkCreateCommandPool(ref->device, &cmd_pool_info, NULL, &ref->cmd_pool);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to create command pool\n // Assertion: `vkCreateCommandPool != VK_SUCCESS`");
		exit(EXIT_FAILURE);
	}

	VkCommandBufferAllocateInfo cmd = {};
	cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd.pNext = NULL;
	cmd.commandPool = ref->cmd_pool;
	cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd.commandBufferCount = 1;

	res = vkAllocateCommandBuffers(ref->device, &cmd, &ref->cmd);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to allocate command buffer\n // Assertion: `vkAllocateCommandBuffers != VK_SUCCESS`");
		exit(EXIT_FAILURE);
	}
}

void cleanup(struct _application *ref)
{
	vkDestroySurfaceKHR(ref->vk_instance, ref->surface, NULL);
	vkDestroyInstance(ref->vk_instance, NULL);

	glfwDestroyWindow(ref->window);
	glfwTerminate();
}
