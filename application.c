#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "application.h"

#include "stdio.h"
#include "stdlib.h"

#ifdef NDEBUG
const uint32_t enable_validation_layers = 0;
#else
const uint32_t enable_validation_layers = 1;
#endif

VkResult create_debug_messenger_EXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) 
{
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != NULL) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void destroy_debug_utils_messenger_EXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != NULL) {
		func(instance, debugMessenger, pAllocator);
	}
}

void populate_debug_messenger_ci(VkDebugUtilsMessengerCreateInfoEXT create_info) 
{
	create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

	create_info.messageSeverity = 
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info.messageType = 
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
}

void setup_debug_messenger(struct _application *ref)
{
	if (enable_validation_layers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT ci = {};
	populate_debug_messenger_ci(ci);

	int res = create_debug_messenger_EXT(ref->vk_instance, &ci, NULL, &ref->debug_messenger);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to create debug messenger \n // Assertion: `create_debug_messenger_EXT() != VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}
}

void query_req_ext(array *arr)
{
	uint32_t glfw_ext_count = 0;
	const char **glfw_extensions;

	glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_ext_count);

	array_init(arr);
	for (uint32_t i = 0; i < glfw_ext_count; i++) {
		array_append(arr, (void *) glfw_extensions[i]);
	}

	if (enable_validation_layers) {
		array_append(arr, (void *) VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
}

int check_validation_layer_support()
{
	uint32_t layer_count;
	vkEnumerateInstanceLayerProperties(&layer_count, NULL);

	array available_layers;
	array_init(&available_layers);
	array_resize(&available_layers, layer_count);

	VkLayerProperties tmp_props[layer_count];
	vkEnumerateInstanceLayerProperties(&layer_count, tmp_props);

	for (uint32_t i = 0; i < layer_count; i++) {

		VkLayerProperties *tmp_layer_prop;
		VkLayerProperties layer_prop_data = tmp_props[i];
		VAL_TO_HEAP(tmp_layer_prop, VkLayerProperties, layer_prop_data);

		array_append(&available_layers, tmp_layer_prop);
	}

	return 0;
}

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
	ref->width = 800;
	ref->height = 600;

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	ref->window = glfwCreateWindow(ref->width, ref->height, "parallax", NULL, NULL);
}

void create_surface(struct _application *ref)
{
	int res = glfwCreateWindowSurface(ref->vk_instance, ref->window, NULL, &ref->surface);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to create window surface via GLFW \n // Assertion: `glfwCreateWindowSurface() != VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}
}

void init_vk(struct _application *ref)
{
	/**
	 * Create VkInstance via filling up CREATE INFO struct.
	 */

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

	/**
	 * Query INSTANCE EXTENSIONS for GLFW etc.
	 */

	array ext_arr;
	query_req_ext(&ext_arr);

	const char * const * tmp_arr[array_size(&ext_arr)];

	for (int i = 0; i < array_size(&ext_arr); i++) {
		tmp_arr[i] = ( ((const char * const *) array_get(&ext_arr, i)));
	}

	instance_ci.enabledExtensionCount = array_size(&ext_arr);
	instance_ci.ppEnabledExtensionNames = *tmp_arr;
	instance_ci.enabledLayerCount = 0;

	int res = vkCreateInstance(&instance_ci, NULL, &ref->vk_instance);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: Failed to instantiate Vulkan\n// Assertion: `vkCreateInstance() == VK_SUCCES`\n");
		exit(EXIT_FAILURE);
	}

	/**
	 * Setup EXTENSION PROPERTIES.
	 */

	uint32_t ext_count = 0;
	vkEnumerateInstanceExtensionProperties(NULL, &ext_count, NULL);
	arr_init(extensions);
	array_resize(&extensions, ext_count);

	VkExtensionProperties tmp_props[ext_count];
	vkEnumerateInstanceExtensionProperties(NULL, &ext_count, tmp_props);

	/**
	 * Finalize by calling the other modules.
	 */

	//create_surface(ref);

	//init_physical_device(ref);
	//init_logical_device(ref);
	//init_swapchain(ref);
}

void init_physical_device(struct _application *ref)
{
	/**
	 * Initialize PHYSICAL DEVICE handle for a possible resource leak reduction.
	 */

	ref->physical_device = VK_NULL_HANDLE;

	/**
	 * Check for physical device candidates eg. graphics cards / integrated systems.
	 * If none is present, abort with corresponding debug-log. 
	 */

	uint32_t dev_count = 0;
	vkEnumeratePhysicalDevices(ref->vk_instance, &dev_count, NULL);

	if (dev_count == 0) {
		fprintf(stderr, "ERR: failed to pick a physical device\n // Assertion: `vkPhysicalDeviceCount > 0`\n");
		exit(EXIT_FAILURE);
	}
	
	/**
	 * Enumerate physical devices and store their handles at heap.
	 */

	array phys_devices;
	VkPhysicalDevice tmp_array[16];

	array_init(&phys_devices);
	array_resize(&phys_devices, dev_count);

	int res = vkEnumeratePhysicalDevices(ref->vk_instance, &dev_count, tmp_array);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to pick a physical device\n // Assertion: `vkEnumeratePhysicalDevices != VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}
	
	arr_append(phys_devices, tmp_array[0]);

	VkPhysicalDevice *p_phys_tmp;
	VkPhysicalDevice phys_tmp = array_get(&phys_devices, 0);
	VAL_TO_HEAP(p_phys_tmp, VkPhysicalDevice, phys_tmp);

	ref->physical_device = *((VkPhysicalDevice *) p_phys_tmp);
}

void init_logical_device(struct _application *ref)
{
	VkResult res;

	VkDeviceQueueCreateInfo queue_info = {};

	uint32_t queue_family_count;
	vkGetPhysicalDeviceQueueFamilyProperties(ref->physical_device, &ref->queue_family_count, NULL);

	printf(">>> %d\n", ref->queue_family_count);

	if (ref->queue_family_count == 0) {
		fprintf(stderr, "ERR: failed to initialize a logical device\n // Assertion: `vkGetPhysicalDeviceQueueFamilyProperties > 0`\n");
		exit(EXIT_FAILURE);
	}

	array_init(&ref->queue_family_properties);
	array_resize(&ref->queue_family_properties, ref->queue_family_count);

	VkQueueFamilyProperties *properties = (VkQueueFamilyProperties *) array_data(&ref->queue_family_properties);

	VkQueueFamilyProperties q_props[ref->queue_family_count];
	vkGetPhysicalDeviceQueueFamilyProperties(ref->physical_device, &ref->queue_family_count, q_props);

	array_resize(&ref->queue_family_properties, ref->queue_family_count);
	for (int i = 0; i < ref->queue_family_count; i++) {
		VkQueueFamilyProperties *p_qprops_tmp;
		VkQueueFamilyProperties qprops_tmp = q_props[i];
		VAL_TO_HEAP(p_qprops_tmp, VkQueueFamilyProperties, qprops_tmp);

		array_append(&ref->queue_family_properties, p_qprops_tmp);
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

	res = vkCreateDevice(ref->physical_device, &device_info, NULL, &ref->device);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to initialize a logical device\n // Assertion: `vkCreateDevice != VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}
}

void create_command_pool(struct _application *ref)
{
	VkResult res;

	/**
	 * Create command bool via filling up CREATE INFO struct.
	 */

	VkCommandPoolCreateInfo cmd_pool_info = {};
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.pNext = NULL;
	cmd_pool_info.queueFamilyIndex = ref->queue_family_index;
	cmd_pool_info.flags = 0;

	res = vkCreateCommandPool(ref->device, &cmd_pool_info, NULL, &ref->cmd_pool);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to create command pool\n // Assertion: `vkCreateCommandPool != VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}

	/**
	 * Create command buffer via filling up CREATE INFO struct.
	 */

	VkCommandBufferAllocateInfo cmd = {};
	cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd.pNext = NULL;
	cmd.commandPool = ref->cmd_pool;
	cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd.commandBufferCount = 1;

	res = vkAllocateCommandBuffers(ref->device, &cmd, &ref->cmd);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to allocate command buffer\n // Assertion: `vkAllocateCommandBuffers != VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}
}

void init_swapchain(struct _application *ref)
{
	VkResult res;

	/**
	 * Iterate over each queue to validate wheter it supports presenting.
	 */
	VkBool32 *p_supports_present = (VkBool32 *) malloc(ref->queue_family_count * sizeof(VkBool32));

	for (uint32_t i = 0; i < ref->queue_family_count; i++) {
		vkGetPhysicalDeviceSurfaceSupportKHR(ref->physical_device, i, ref->surface, &p_supports_present[i]);
	}

	/**
	 * Search for GRAPHICS and PRESENT queues in the corresponding array,
	 * trying to find one that supports both.
	 */
	ref->graphics_queue_family_index = UINT32_MAX;
	ref->present_queue_family_index = UINT32_MAX;

	printf("- %d\n", ref->queue_family_count);

	for (uint32_t i = 0; i < ref->queue_family_count; ++i) {
		VkQueueFamilyProperties tmp_props = (*(VkQueueFamilyProperties *) array_get(&ref->queue_family_properties, i));
		printf("queue_flags <<< %d\n", (*(VkQueueFamilyProperties *) array_get(&ref->queue_family_properties, i)).queueFlags);
		if ((tmp_props.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {

			printf("yoa\n");

			if (ref->queue_family_index == UINT32_MAX)
				ref->graphics_queue_family_index = i;

			if (p_supports_present[i] == VK_TRUE) {
				ref->graphics_queue_family_index = i;
				ref->present_queue_family_index = i;
				break;
			}
		}
	}

	if (ref->present_queue_family_index == UINT32_MAX) {
		/**
		 * If a queue that supports both GRAPHICS and PRESENT,
		 * find a separate PRESENT queue.
		 */
		for (size_t i = 0; i < ref->queue_family_count; ++i) {
			if (p_supports_present[i] == VK_TRUE) {
				ref->present_queue_family_index = i;
				break;
			}
		}
	}

	free(p_supports_present);

	/**
	 * Throw an error if GRAPHICS and PRESENT supporting queues was not present.
	 */

	printf("yö\n");

	if (ref->graphics_queue_family_index == UINT32_MAX || ref->present_queue_family_index == UINT32_MAX) {
		fprintf(stderr, "ERR: Could not find a queue supporting GRAPHICS and PRESENT\n");
		exit(EXIT_FAILURE);
	}

	printf("ss\n");

	/**
	 * Get list of VkFormats that are supported.
	 */

	uint32_t format_count;
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(ref->physical_device, ref->surface, &format_count, NULL);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to query for VkFormats\n // Assertion: `vkGetPhysicalDeviceSurfaceFormatsKHR != VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}

	VkSurfaceFormatKHR *surf_formats = (VkSurfaceFormatKHR *) malloc(format_count * sizeof(VkSurfaceFormatKHR));
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(ref->physical_device, ref->surface, &format_count, surf_formats);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to query for VkFormats\n // Assertion: `vkGetPhysicalDeviceSurfaceFormatsKHR != VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}

	/**
	 * If the format list includes only one entry of VK_FORMAT_UNDEFINED,
	 * the surface has no preferred format. Otherwise, at least one
	 * supported format will be returned.
	 */

	if (format_count == 1 && surf_formats[0].format == VK_FORMAT_UNDEFINED) {
		ref->format = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else {
		if (!(format_count >= 1)) {
			fprintf(stderr, "ERR: failed to query for VkFormats\n // Assertion: `VkFormat - count < 1`\n");
			exit(EXIT_FAILURE);
		}

		ref->format = surf_formats[0].format;		
	}

	printf("aaa\n");

	free(surf_formats);

	printf("aaa*\n");

	VkSurfaceCapabilitiesKHR surf_capabilities;

	printf("bbb\n");

	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ref->physical_device, ref->surface, &surf_capabilities);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to query for Surface Capabilities\n // Assertion: `vkGetPhysicalDeviceSurfaceCapabilitiesKHR != VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}

	printf("bbb*\n");

	uint32_t present_mode_count;
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(ref->physical_device, ref->surface, &present_mode_count, NULL);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to query for Present Modes\n // Assertion: `vkGetPhysicalDeviceSurfacePresentModesKHR != VK_SUCCESS`");
		exit(EXIT_FAILURE);
	}

	printf("bbb**\n");

	VkPresentModeKHR *present_modes = (VkPresentModeKHR *) malloc(present_mode_count * sizeof(VkPresentModeKHR));

	res = vkGetPhysicalDeviceSurfacePresentModesKHR(ref->physical_device, ref->surface, &present_mode_count, present_modes);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to query for Present Modes (*)\n // Assertion: `vkGetPhysicalDeviceSurfacePresentModesKHR != VK_SUCCESS`");
		exit(EXIT_FAILURE);
	}

	printf("bbb(*)\n");

	/**
	 * Define swapchain extents
	 */

	printf("ccc\n");

	VkExtent2D swapchain_extent;
	if (surf_capabilities.currentExtent.width == 0xFFFFFFFF) {
		swapchain_extent.width = ref->width;
		swapchain_extent.height = ref->height;

		if (swapchain_extent.width < surf_capabilities.minImageExtent.width) {
			swapchain_extent.width = surf_capabilities.minImageExtent.width;
		}
		else if (swapchain_extent.width > surf_capabilities.maxImageExtent.width) {
			swapchain_extent.width = surf_capabilities.maxImageExtent.width;
		}

		if (swapchain_extent.height < surf_capabilities.minImageExtent.height) {
			swapchain_extent.height = surf_capabilities.minImageExtent.height;
		}
		else if (swapchain_extent.height > surf_capabilities.maxImageExtent.height) {
			swapchain_extent.height = surf_capabilities.maxImageExtent.height;
		}
	}
	else {
		swapchain_extent = surf_capabilities.currentExtent;
	}

	VkPresentModeKHR swapchain_present_mode = VK_PRESENT_MODE_FIFO_KHR;

	/**
	 * Determine the number of VkImage's to use in the swapchain.
	 * Only one presentable image is required at the time.
	 */

	printf("ddd\n");

	uint32_t desiredNumberOfSwapChainImages = surf_capabilities.minImageCount;

    	VkSurfaceTransformFlagBitsKHR pre_transform;
   	if (surf_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
       		pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  	} else {
  	      pre_transform = surf_capabilities.currentTransform;
   	}

   	printf("eee\n");

   	/**
   	 * Find a supported composite alpha mode - one of following is guaranteed to be set.
   	 */

	VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	VkCompositeAlphaFlagBitsKHR composite_alpha_flags[4] = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, 
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR, 
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR, 
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};

	printf("fff\n");

	for (uint32_t i = 0; i < sizeof(composite_alpha_flags) / sizeof(composite_alpha_flags[0]); i++) {
		if (surf_capabilities.supportedCompositeAlpha & composite_alpha_flags[i]) {
			composite_alpha = composite_alpha_flags[i];
			break;
		}
	}

	VkSwapchainCreateInfoKHR swapchain_ci = {};
	swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_ci.pNext = NULL;
	swapchain_ci.surface = ref->surface;
	swapchain_ci.minImageCount = desiredNumberOfSwapChainImages;
	swapchain_ci.imageFormat = ref->format;
	swapchain_ci.imageExtent.width = swapchain_extent.width;
	swapchain_ci.imageExtent.height = swapchain_extent.height;
	swapchain_ci.preTransform = pre_transform;
	swapchain_ci.compositeAlpha = composite_alpha;
	swapchain_ci.imageArrayLayers = 1;
	swapchain_ci.presentMode = swapchain_present_mode;
	swapchain_ci.oldSwapchain = VK_NULL_HANDLE;
	swapchain_ci.clipped = (VkBool32) 1;
	swapchain_ci.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_ci.queueFamilyIndexCount = 0;
	swapchain_ci.pQueueFamilyIndices = NULL;

	printf("ggg\n");

	uint32_t queue_family_indices[2] = { (uint32_t) ref->graphics_queue_family_index, (uint32_t) ref->present_queue_family_index };

	printf("g < %d, p < %d\n", ref->graphics_queue_family_index, ref->present_queue_family_index);

	if (ref->graphics_queue_family_index != ref->present_queue_family_index) {

		printf("yöss\n");

		swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchain_ci.queueFamilyIndexCount = 2;
		swapchain_ci.pQueueFamilyIndices = queue_family_indices;
	}

	printf("ggg*\n");

	res = vkCreateSwapchainKHR(ref->device, &swapchain_ci, NULL, &ref->swapchain);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed initialize swapchain\n // Assertion: `vkCreateSwapchainKHR != VK_SUCCESS`");
		exit(EXIT_FAILURE);
	}

	printf("ggg**\n");

	res = vkGetSwapchainImagesKHR(ref->device, ref->swapchain, &ref->swapchain_img_count, NULL);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed get swapchain images\n // Assertion: `vkGetSwapchainImagesKHR != VK_SUCCESS`");
		exit(EXIT_FAILURE);
	}

	printf("ggg(*)\n");

	VkImage *swapchain_imgs = (VkImage *) malloc(ref->swapchain_img_count * sizeof(VkImage));
	res = vkGetSwapchainImagesKHR(ref->device, ref->swapchain, &ref->swapchain_img_count, &ref->swapchain_img);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed get swapchain images (*)\n // Assertion: `vkGetSwapchainImagesKHR != VK_SUCCESS`");
		exit(EXIT_FAILURE);
	}

	printf("ggg(*)*\n");

	array_resize(&ref->swapc_buffer, ref->swapchain_img_count);
	for (uint32_t i = 0; i < ref->swapchain_img_count; i++) {
		swapchain_buffer sc_buffer;

		swapchain_buffer *p_scbuffer_tmp;
		swapchain_buffer scbuffer;
		scbuffer.image = swapchain_imgs[i];
		scbuffer.view = VK_NULL_HANDLE;

		VAL_TO_HEAP(p_scbuffer_tmp, swapchain_buffer, scbuffer);
		array_append(&ref->swapc_buffer, p_scbuffer_tmp);
	}

	free(swapchain_imgs);

	printf("ggg(**)\n");

	for (uint32_t i = 0; i < ref->swapchain_img_count; i++) {
		VkImageViewCreateInfo color_image_view = {};
		color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		color_image_view.pNext = NULL;
		color_image_view.flags = 0;
		color_image_view.image = *((VkImage *) array_get(&ref->swapc_buffer, i));
		color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		color_image_view.format = ref->format;
		color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;
		color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;
		color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;
		color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;
		color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		color_image_view.subresourceRange.baseMipLevel = 0;
		color_image_view.subresourceRange.levelCount = 1;
		color_image_view.subresourceRange.baseArrayLayer = 0;
		color_image_view.subresourceRange.layerCount = 1;

		res = vkCreateImageView(ref->device, &color_image_view, NULL, array_get(&ref->swapc_buffer, i));
		if (res != VK_SUCCESS) {
			fprintf(stderr, "ERR: failed to initialize Image Views\n // Assertion: `vkCreateImageView != VK_SUCCESS`\n");
			exit(EXIT_FAILURE);
		}
	}

	printf("youy\n");

}


void cleanup(struct _application *ref)
{
	for (uint32_t i = 0; i < ref->swapchain_img_count; i++) {

		swapchain_buffer tmp_view = *((swapchain_buffer *) array_get(&ref->swapc_buffer, i));
		vkDestroyImageView(ref->device, tmp_view.view, NULL);
	}

	vkDestroySurfaceKHR(ref->vk_instance, ref->surface, NULL);
	vkDestroyInstance(ref->vk_instance, NULL);

	glfwDestroyWindow(ref->window);
	glfwTerminate();
}
