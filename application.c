#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "application.h"

#include "validations.h"
#include "swapc.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include <string.h>

#ifdef NDEBUG
const uint32_t enable_validation_layers = 0;
#else
const uint32_t enable_validation_layers = 1;
#endif

const char *p_extensions[1] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
const char *p_layers[1] = { "VK_LAYER_KHRONOS_validation" };

const int MAX_FRAMES_IN_FLIGHT = 2;
size_t current_frame = 0;



/**
 *	Raw vertex data waiting for Vertex / Uniform buffer implementation.
 *	Given pos (x, y) and color data (rgb) in form of vertex_t struct.
 */
const struct vertex_t vertices[] = {
	{{0.0f, -0.5f}, {1.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f},  {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}}
};

/**
 *	Create and get associated `VkVertexInputBindingDescription` for
 *	vertex buffer creation in graphics pipeline.  
 */
static VkVertexInputBindingDescription get_binding_description() 
{
	VkVertexInputBindingDescription binding_description = {};
	binding_description.binding = 0;
	binding_description.stride = sizeof(vertex_t);
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return binding_description;
}

/**
 *	Create and get associated `VkVertexInputAttributeDescription` for
 *	vertex buffer creation in graphics pipeline.
 */
static array get_attribute_description()
{
	array attrib_desc;
	array_init(&attrib_desc, sizeof(VkVertexInputAttributeDescription));
	array_resize(&attrib_desc, 2, false);

	VkVertexInputAttributeDescription attrib_arr[2] = {};

	attrib_arr[0].binding = 0;
	attrib_arr[0].location = 0;
	attrib_arr[0].format = VK_FORMAT_R32G32_SFLOAT;
	attrib_arr[0].offset = offsetof(vertex_t, pos);

	attrib_arr[1].binding = 0;
	attrib_arr[1].location = 1;
	attrib_arr[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attrib_arr[1].offset = offsetof(vertex_t, color);

	for (int i = 0; i < 2; i++) {

		array_append(&attrib_desc, &attrib_arr[i]);
	}

	return attrib_desc;
}


/**
 *	Determine wheter the given physical device has the required extensions to
 *	be used in the application.
 *	
 *	return true (1) if supported, false (0) otherwise. 
 */
bool check_dev_ext_support(VkPhysicalDevice phys_device)
{
	uint32_t ext_count;
	vkEnumerateDeviceExtensionProperties(phys_device, NULL, &ext_count, NULL);

	VkExtensionProperties available_ext[ext_count];
	vkEnumerateDeviceExtensionProperties(phys_device, NULL, &ext_count, available_ext);

	int count = 0;
	for (int i = 0; i < (sizeof(p_extensions) / sizeof(const char *)); i++) {
		for (int j = 0; j < ext_count; j++) {

			if (strcmp(available_ext[j].extensionName, p_extensions[i]) == 0) {
				count++;
			}
		}
	}

	return count == (int) (sizeof(p_extensions) / sizeof(const char *));
}


/**
 *	Determine wheter the given physical device is suitable
 *	to be used in the application by its capabilities in queue family / swapchain support.
 */
bool is_dev_suitable(VkPhysicalDevice phys_device, struct _application *ref)
{

	queue_family_indices_t indices = {-1};
	indices = query_queue_families(phys_device, ref->surface);

	bool indices_supp = false;
	bool ext_supp = check_dev_ext_support(phys_device);

	if (indices.graphics_family != -1) {
		indices_supp = true;
	}

	bool swp_adequate = false;

	if (ext_supp) {
		swapchain_supp_detail_t swp_supp = query_swapchain_supp(phys_device, ref->surface);
		swp_adequate = !(array_size(&swp_supp.formats) == 0) && !(array_size(&swp_supp.present_modes) == 0);
	}
	
	return indices_supp && ext_supp && swp_adequate;
}

/**
 *	Query the required extensions for the instance and device and pass them back
 *	to the given array.
 */
void query_req_ext(array *arr)
{
	uint32_t glfw_ext_count = 0;
	const char **glfw_extensions;

	glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_ext_count);

	array_init(arr, sizeof(const char[256]));

	for (uint32_t i = 0; i < glfw_ext_count; i++) {
		array_append(arr, (void *) glfw_extensions[i]);
	}

	if (enable_validation_layers) {
		array_append(arr, (void *) VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
}

/**
 *	Setup the debug / validation layer functionality to this specific instance.
 */

void setup_debug_messenger(struct _application *ref)
{
	if (!enable_validation_layers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT ci;
	populate_debug_messenger_ci(&ci);

	int res = create_debug_messenger_EXT(ref->vk_instance, &ci, NULL, &ref->debug_messenger);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to create debug messenger \n // Assertion: `create_debug_messenger_EXT() != VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}
}

/**
 *	A general util to find physical devices queue family support.
 */

queue_family_indices_t query_queue_families(VkPhysicalDevice phys_device, VkSurfaceKHR surface)
{
	queue_family_indices_t indices = {VK_NULL_HANDLE, VK_NULL_HANDLE};

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_family_count, NULL);

	VkQueueFamilyProperties queue_families[queue_family_count];
	vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_family_count, queue_families);

	for (int i = 0; i < queue_family_count; i++) {
		
		VkQueueFamilyProperties props = queue_families[i];

		if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphics_family = i;
		}

		VkBool32 present_supp;
		vkGetPhysicalDeviceSurfaceSupportKHR(phys_device, i, surface, &present_supp);

		if (present_supp) {
			indices.present_family = i;
		}

		if ((indices.graphics_family == VK_NULL_HANDLE) && (indices.present_family == VK_NULL_HANDLE)) {
			break;
		}
	}

	return indices;
}

/**
 *	Bootstrap function to be called to start the application.
 */
void run(struct _application *ref)
{
	init_window(ref);
	init_vk(ref);

	main_loop(ref);

	cleanup(ref);
}


/**
 *	Main loop to be iterated while the application is running, eg. window is not closed.
 */
void main_loop(struct _application *ref)
{
	while (!glfwWindowShouldClose(ref->window))
	{
		glfwPollEvents();
		draw_frame(ref);
	}

	vkDeviceWaitIdle(ref->device);
}

/**
 *	Get an image from the swapchain and render a frame to that.
 */
void draw_frame(struct _application *ref) 
{
	VkResult res;

	vkWaitForFences(ref->device, 1, &arr_get(ref->in_flight_fences, VkFence, current_frame), VK_TRUE, UINT64_MAX);

	uint32_t img_index;
	res = vkAcquireNextImageKHR(ref->device, ref->swapchain, UINT64_MAX, arr_get(ref->img_available_semaphore, VkSemaphore, current_frame), VK_NULL_HANDLE, &img_index);

	if (res == VK_ERROR_OUT_OF_DATE_KHR) {
		recreate_swapchain(ref);
	}
	else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
		fprintf(stderr, "ERR: failed to acquire swapchain image \n // Assertion: `vkAcquireNextImageKHR() != VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}

	if (array_get(&ref->imgs_in_flight, img_index) != NULL) {
		printf("jojojojo\n");
		vkWaitForFences(ref->device, 1, &arr_get(ref->imgs_in_flight, VkFence, img_index), VK_TRUE, UINT64_MAX);
	}

	array_set(&ref->imgs_in_flight, current_frame, array_get(&ref->in_flight_fences, current_frame));

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore wait_semaphores[] = { arr_get(ref->img_available_semaphore, VkSemaphore, current_frame) };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;

	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &arr_get(ref->cmd_buffers, VkCommandBuffer, img_index);

	VkSemaphore signal_semaphores[] = { arr_get(ref->render_finished_semaphore, VkSemaphore, current_frame) };
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;

	vkResetFences(ref->device, 1, &arr_get(ref->in_flight_fences, VkFence, current_frame));

	res = vkQueueSubmit(ref->graphics_queue, 1, &submit_info, arr_get(ref->in_flight_fences, VkFence, current_frame));
	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || ref->framebuffer_resized) {
		ref->framebuffer_resized = false;
		recreate_swapchain(ref);
	}
	else if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to submit queue \n // Assertion: `vkQueueSubmit() != VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;

	VkSwapchainKHR swapchains[] = { ref->swapchain };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapchains;
	present_info.pImageIndices = &img_index;


	vkQueuePresentKHR(ref->present_queue, &present_info);

	current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

/**
 *	A callback to be called by GLFW when the window surface is resized.
 */
static void framebuffer_resize_callback(GLFWwindow *window, int width, int height)
{
	application *app = (application *) glfwGetWindowUserPointer(window);
	app->framebuffer_resized = true;
}

/**
 *	Initialize GLFW to be used as an abstraction for WSI cross compability.
 */
void init_window(struct _application *ref)
{
	ref->width = 800;
	ref->height = 600;

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	ref->window = glfwCreateWindow(ref->width, ref->height, "parallax", NULL, NULL);
	glfwSetWindowUserPointer(ref->window, ref);
	glfwSetFramebufferSizeCallback(ref->window, framebuffer_resize_callback);
}

/**
 *	Create a surface from GLFW to be used as a handle in Vulkan instance.
 */
void create_surface(struct _application *ref)
{
	int res = glfwCreateWindowSurface(ref->vk_instance, ref->window, NULL, &ref->surface);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to create window surface via GLFW \n // Assertion: `glfwCreateWindowSurface() != VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}
}

/**
 *	Create the Vulkan synchronizing objects required to 
 *	maintain connection between (GPU <-> CPU) and (GPU <-> GPU)
 */
void create_sync_objects(struct _application *ref)
{

	array_init(&ref->img_available_semaphore, sizeof(VkSemaphore));
	array_init(&ref->render_finished_semaphore, sizeof(VkSemaphore));
	array_init(&ref->in_flight_fences, sizeof(VkFence));
	array_init(&ref->imgs_in_flight, sizeof(VkFence));

	array_resize(&ref->img_available_semaphore, MAX_FRAMES_IN_FLIGHT, true);
	array_resize(&ref->render_finished_semaphore, MAX_FRAMES_IN_FLIGHT, true);
	array_resize(&ref->in_flight_fences, MAX_FRAMES_IN_FLIGHT, true);
	array_resize(&ref->imgs_in_flight, array_size(&ref->swapc_imgs), false);

	VkSemaphoreCreateInfo semaphore_ci = {};
	semaphore_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_ci = {};
	fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSemaphore tmp[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore tmp0[MAX_FRAMES_IN_FLIGHT];
	VkFence tmp1[MAX_FRAMES_IN_FLIGHT];

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

		VkResult res = vkCreateSemaphore(ref->device, &semaphore_ci, NULL, &((VkSemaphore *) array_data(&ref->img_available_semaphore))[i]);
		if (res != VK_SUCCESS) {
			fprintf(stderr, "ERR: failed to create semaphore for available images \n // Assertion: `vkCreateSemaphore() != VK_SUCCESS`\n");
			exit(EXIT_FAILURE);
		}

		res = vkCreateSemaphore(ref->device, &semaphore_ci, NULL, &((VkSemaphore *) array_data(&ref->render_finished_semaphore))[i]);
		if (res != VK_SUCCESS) {
			fprintf(stderr, "ERR: failed to create semaphore for finished render \n // Assertion: `vkCreateSemaphore() != VK_SUCCESS`\n");
			exit(EXIT_FAILURE);
		}

		res = vkCreateFence(ref->device, &fence_ci, NULL, &((VkFence *) array_data(&ref->in_flight_fences))[i]);
		if (res != VK_SUCCESS) {
			fprintf(stderr, "ERR: failed to create fences \n // Assertion: `vkCreateFence() != VK_SUCCESS`\n");
			exit(EXIT_FAILURE);
		}
	}
}

/**
 *	Initialize all Vulkan stuff.
 */
void init_vk(struct _application *ref)
{
	VkResult res;

	ref->framebuffer_resized = false;

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
	app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instance_ci = {};
	instance_ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_ci.pApplicationInfo = &app_info;

	/**
	 * Query INSTANCE EXTENSIONS for GLFW etc.
	 */

	array ext_arr;
	query_req_ext(&ext_arr);

	const char *tmp_arr[array_size(&ext_arr)];

	for (int i = 0; i < array_size(&ext_arr); i++) {
		tmp_arr[i] = ( ((const char *) array_get(&ext_arr, i)));
	}

	instance_ci.enabledExtensionCount = (uint32_t) array_size(&ext_arr);
	instance_ci.ppEnabledExtensionNames = tmp_arr;

	VkDebugUtilsMessengerCreateInfoEXT debug_ci;
	if (enable_validation_layers) {
		instance_ci.enabledLayerCount = 1;
		instance_ci.ppEnabledLayerNames = p_layers;

		populate_debug_messenger_ci(&debug_ci);
		instance_ci.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debug_ci;
	}
	else {
		instance_ci.enabledLayerCount = 0;
		instance_ci.pNext = NULL;
	}

	res = vkCreateInstance(&instance_ci, NULL, &ref->vk_instance);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: Failed to instantiate Vulkan\n// Assertion: `vkCreateInstance() == VK_SUCCES`\n");
		exit(EXIT_FAILURE);
	}

	/**
	 * Setup EXTENSION PROPERTIES.
	 */

	uint32_t ext_count = 0;
	vkEnumerateInstanceExtensionProperties(NULL, &ext_count, NULL);

	array extensions;
	array_init(&extensions, sizeof(VkExtensionProperties));

	array_resize(&extensions, ext_count, true);

	vkEnumerateInstanceExtensionProperties(NULL, &ext_count, (VkExtensionProperties *) array_data(&extensions));

	setup_debug_messenger(ref);

	/**
	 * Finalize by calling the other modules.
	 */

	create_surface(ref);

	init_physical_device(ref);
	init_logical_device(ref);

	init_swapchain(ref);
	init_image_views(ref);

	create_renderpass(ref);
	create_graphics_pipeline(ref);
	create_framebuffers(ref);
	create_command_pool(ref);
	create_vertex_buffer(ref);
	create_command_buffers(ref);
	create_sync_objects(ref);

}

/**
 *	Initialize physical devices to be used.
 */
void init_physical_device(struct _application *ref)
{

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

	array_init(&ref->physical_devices, sizeof(VkPhysicalDevice));
	array_resize(&ref->physical_devices, dev_count, true);

	int res = vkEnumeratePhysicalDevices(ref->vk_instance, &dev_count, (VkPhysicalDevice *) array_data(&ref->physical_devices));
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to pick a physical device\n // Assertion: `vkEnumeratePhysicalDevices == VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}
}

/**
 *	Initialize a logical device to be used in Vulkan.
 */
void init_logical_device(struct _application *ref)
{
	VkResult res;

	queue_family_indices_t indices = query_queue_families(PHYSDEV(0), ref->surface);

	VkDeviceQueueCreateInfo queue_infos[2];
	uint32_t unique_queue_families[2] = {indices.graphics_family, indices.present_family};

	float queue_priority = 1.0f;

	for (int i = 0; i < 2; i++) {
		VkDeviceQueueCreateInfo queue_ci = {};

		queue_ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_ci.queueFamilyIndex = i;
		queue_ci.queueCount = 1;
		queue_ci.pQueuePriorities = &queue_priority;

		queue_infos[i] = queue_ci;
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo device_info = {};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pQueueCreateInfos = queue_infos;
	device_info.queueCreateInfoCount = 1;

	device_info.pEnabledFeatures = &deviceFeatures;

	device_info.enabledExtensionCount = 1;
	device_info.ppEnabledExtensionNames = p_extensions;

	if (enable_validation_layers) {
		device_info.enabledLayerCount = 1;
		device_info.ppEnabledLayerNames = p_layers;
	}
	else {
		device_info.enabledLayerCount = 0;
	}

	res = vkCreateDevice(PHYSDEV(0), &device_info, NULL, &ref->device);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to initialize a logical device\n // Assertion: `vkCreateDevice != VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}

	vkGetDeviceQueue(ref->device, indices.graphics_family, 0, &ref->graphics_queue);
	vkGetDeviceQueue(ref->device, indices.present_family, 0, &ref->present_queue);
}


/**
 *	Create the command pool to be used in creation and reuse of command buffers.
 */
void create_command_pool(struct _application *ref)
{
	queue_family_indices_t queue_family_indices = query_queue_families(PHYSDEV(0), ref->surface);

	VkCommandPoolCreateInfo commandpool_ci = {};
	commandpool_ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandpool_ci.queueFamilyIndex = queue_family_indices.graphics_family;

	VkResult res = vkCreateCommandPool(ref->device, &commandpool_ci, NULL, &ref->cmd_pool);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to initialize command buffer\n // Assertion: `vkCreateCommandPool != VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}
}

/**
 *	Create command buffers to be recorded.
 */
void create_command_buffers(struct _application *ref)
{
	array_init(&ref->cmd_buffers, sizeof(VkCommandBuffer));
	array_resize(&ref->cmd_buffers, array_size(&ref->swapc_framebuffers), true);

	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = ref->cmd_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = (uint32_t) array_size(&ref->cmd_buffers);

	VkCommandBuffer tmp[array_size(&ref->swapc_framebuffers)];

	VkResult res = vkAllocateCommandBuffers(ref->device, &alloc_info, (VkCommandBuffer *) array_data(&ref->cmd_buffers));
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to initialize command buffer\n // Assertion: `vkCreateCommandPool != VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < array_size(&ref->cmd_buffers); i++) {

		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = 0;
		begin_info.pInheritanceInfo = NULL;

		res = vkBeginCommandBuffer(arr_get(ref->cmd_buffers, VkCommandBuffer, i), &begin_info);
		if (res != VK_SUCCESS) {
			fprintf(stderr, "ERR: failed to initialize command buffer\n // Assertion: `vkCreateCommandPool != VK_SUCCESS`\n");
			exit(EXIT_FAILURE);
		}

		VkOffset2D offset = {0, 0};
		VkClearValue clear_color = {0.0f, 0.0f, 0.0f, 0.0f};

		VkRenderPassBeginInfo render_pass_bi = {};
		render_pass_bi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_bi.renderPass = ref->render_pass;
		render_pass_bi.framebuffer = arr_get(ref->swapc_framebuffers, VkFramebuffer, i);
		render_pass_bi.renderArea.offset = offset;
		render_pass_bi.renderArea.extent = ref->swapc_extent;

		render_pass_bi.clearValueCount = 1;
		render_pass_bi.pClearValues = &clear_color;

		vkCmdBeginRenderPass(arr_get(ref->cmd_buffers, VkCommandBuffer, i), &render_pass_bi, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(arr_get(ref->cmd_buffers, VkCommandBuffer, i), VK_PIPELINE_BIND_POINT_GRAPHICS, ref->graphics_pipeline);

		VkBuffer vertex_buffers[] = { ref->vertex_buffer };
		VkDeviceSize offsets[] = {0}; 
		vkCmdBindVertexBuffers(arr_get(ref->cmd_buffers, VkCommandBuffer, i), 0, 1, vertex_buffers, offsets);

		vkCmdDraw(arr_get(ref->cmd_buffers, VkCommandBuffer, i), sizeof(vertices), 1, 0, 0);
		vkCmdEndRenderPass(arr_get(ref->cmd_buffers, VkCommandBuffer, i));

		res = vkEndCommandBuffer(arr_get(ref->cmd_buffers, VkCommandBuffer, i));
		if (res != VK_SUCCESS) {
			fprintf(stderr, "ERR: failed to record command buffer\n // Assertion: `vkCmdEndRenderPass != VK_SUCCESS`\n");
			exit(EXIT_FAILURE);
		}

	}

}

/**
 *	Initialize image views for images to be accessed in graphics pipeline
 */
void init_image_views(struct _application *ref)
{

	array_init(&ref->swapc_img_views, sizeof(VkImageView));
	array_resize(&ref->swapc_img_views, array_size(&ref->swapc_imgs), true);

	VkImageView tmp[array_size(&ref->swapc_imgs)];

	for (int i = 0; i < array_size(&ref->swapc_imgs); i++) {
		VkImageViewCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ci.image = arr_get(ref->swapc_imgs, VkImage, i);
		ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ci.format = ref->swapc_img_format;

		ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ci.subresourceRange.baseMipLevel = 0;
		ci.subresourceRange.levelCount = 1;
		ci.subresourceRange.baseArrayLayer = 0;
		ci.subresourceRange.layerCount = 1;

		VkResult res = vkCreateImageView(ref->device, &ci, NULL, &((VkImageView *) array_data(&ref->swapc_img_views))[i]);
		if (res != VK_SUCCESS) {
			fprintf(stderr, "ERR: failed to initialize image views\n // Assertion: `vkCreateImageView != VK_SUCCESS`\n");
			exit(EXIT_FAILURE);
		}
	}
}

/**
 *	Create a shader module from a SPIR-V binary file.
 */
void create_shader_from_file(const char *file_path, VkShaderModule *shader, VkDevice dev)
{
	VkResult res;

	void *code = NULL;

	size_t size = 0;
	size_t cur = 0;

	FILE *f_in = fopen(file_path, "rb");

	*shader = NULL;

	if (f_in == NULL) {
		fprintf(stderr, "ERR: failed to read shader file\n // Assertion: `shader_file != NULL`\n");
		exit(EXIT_FAILURE);
	}

	fseek(f_in, 0, SEEK_END);
	size = ftell(f_in);
	fseek(f_in, 0, SEEK_SET);

	code = malloc(size);
	if (code == NULL) {
		fprintf(stderr, "ERR: failed to read shader file\n // Assertion: `malloc != NULL`\n");
		exit(EXIT_FAILURE);
	}

	while(cur < size) {

		size_t read = fread(code + cur, 1, size - cur, f_in);
		if (read == 0) {
			fprintf(stderr, "ERR: failed to read shader file\n // Assertion: `read != 0`\n");
			exit(EXIT_FAILURE);
		}

		cur += read;
	}

	VkShaderModuleCreateInfo shader_ci = {};

	shader_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_ci.codeSize = size;
	shader_ci.pCode = code;


	res = vkCreateShaderModule(dev, &shader_ci, NULL, shader);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to initialize image views\n // Assertion: `vkCreateImageView != VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}
}

/**
 *	Create a render pass and derived subpasses. 
 */
void create_renderpass(struct _application *ref)
{
	VkResult res;

	VkAttachmentDescription color_attachment = {};
	color_attachment.format = ref->swapc_img_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo render_pass_ci = {};
	render_pass_ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_ci.attachmentCount = 1;
	render_pass_ci.pAttachments = &color_attachment;
	render_pass_ci.subpassCount = 1;
	render_pass_ci.pSubpasses = &subpass;
	render_pass_ci.dependencyCount = 1;
	render_pass_ci.pDependencies = &dependency;

	res = vkCreateRenderPass(ref->device, &render_pass_ci, NULL, &ref->render_pass);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to initialize render pass\n // Assertion: `vkCreateImageView != VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}
}

uint32_t find_memory_type(struct _application *ref, uint32_t type_filter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties mem_props;
	vkGetPhysicalDeviceMemoryProperties(PHYSDEV(0), &mem_props);

	for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
		if ((type_filter & (1 << i)) && (mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	return VK_NULL_HANDLE;
}

void copy_buffer(struct _application *ref, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) 
{
	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandPool = ref->cmd_pool;
	alloc_info.commandBufferCount = 1;

	VkCommandBuffer command_buffer;
	vkAllocateCommandBuffers(ref->device, &alloc_info, &command_buffer);

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(command_buffer, &begin_info);

	VkBufferCopy copy_region = {};
	copy_region.srcOffset = 0;
	copy_region.dstOffset = 0;
	copy_region.size = size;
	vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

	vkEndCommandBuffer(command_buffer);

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	vkQueueSubmit(ref->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(ref->graphics_queue);

	vkFreeCommandBuffers(ref->device, ref->cmd_pool, 1, &command_buffer);
}

void create_buffer(struct _application *ref, VkDeviceSize size, VkBufferUsageFlags usage, 
	VkMemoryPropertyFlags props, VkBuffer *buffer, VkDeviceMemory *buffer_mem)
{
	VkResult res;

	VkBufferCreateInfo buffer_info = {};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = size;
	buffer_info.usage = usage;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	res = vkCreateBuffer(ref->device, &buffer_info, NULL, buffer);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to create buffer\n // Assertion: `vkCreateBuffer != VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}

	VkMemoryRequirements mem_req;
	vkGetBufferMemoryRequirements(ref->device, *buffer, &mem_req);

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_req.size;
	alloc_info.memoryTypeIndex = find_memory_type(ref, mem_req.memoryTypeBits, props);

	res = vkAllocateMemory(ref->device, &alloc_info, NULL, buffer_mem);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to allocate buffer memory\n // Assertion: `vkAllocateMemory != VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}

	vkBindBufferMemory(ref->device, *buffer, *buffer_mem, 0);
}

void create_vertex_buffer(struct _application *ref)
{
	VkDeviceSize buffer_size = sizeof(vertices);

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	create_buffer(ref, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_buffer_memory);

	void *data;
	vkMapMemory(ref->device, staging_buffer_memory, 0, buffer_size, 0, &data);
	memcpy(data, vertices, (size_t) buffer_size);
	vkUnmapMemory(ref->device, staging_buffer_memory);

	create_buffer(ref, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &ref->vertex_buffer, &ref->vertex_buffer_memory);
	copy_buffer(ref, staging_buffer, ref->vertex_buffer, buffer_size);

	vkDestroyBuffer(ref->device, staging_buffer, NULL);
	vkFreeMemory(ref->device, staging_buffer_memory, NULL);
}

/**
 *	Create the needed graphics pipeline stuff.
 */
void create_graphics_pipeline(struct _application *ref)
{
	VkResult res;

	VkShaderModule vert;
	VkShaderModule frag;

	create_shader_from_file("shaders/vert.spv", &vert, ref->device);
	create_shader_from_file("shaders/frag.spv", &frag, ref->device);

	VkPipelineShaderStageCreateInfo vert_shader_stage_ci = {};

	vert_shader_stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_shader_stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_shader_stage_ci.module = vert;
	vert_shader_stage_ci.pName = "main";

	VkPipelineShaderStageCreateInfo frag_shader_stage_ci = {};

	frag_shader_stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_shader_stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_shader_stage_ci.module = frag;
	frag_shader_stage_ci.pName = "main";

	VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_ci, frag_shader_stage_ci};

	VkPipelineVertexInputStateCreateInfo vert_input_info = {};

	VkVertexInputBindingDescription bind_desc = get_binding_description();
	array attrib_desc = get_attribute_description();

	VkVertexInputAttributeDescription attr_tmp[array_size(&attrib_desc)];
	for (int i = 0; i < array_size(&attrib_desc); i++) {
		attr_tmp[i] = arr_get(attrib_desc, VkVertexInputAttributeDescription, i);
	}

	vert_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vert_input_info.vertexBindingDescriptionCount = 1;
	vert_input_info.vertexAttributeDescriptionCount = (uint32_t) array_size(&attrib_desc);
	vert_input_info.pVertexBindingDescriptions = &bind_desc;
	vert_input_info.pVertexAttributeDescriptions = attr_tmp;

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};

	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) ref->swapc_extent.width;
	viewport.height = (float) ref->swapc_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkOffset2D offset = {0, 0};

	VkRect2D scissor = {};
	scissor.offset = offset;
	scissor.extent = ref->swapc_extent;

	VkPipelineViewportStateCreateInfo viewport_state = {};

	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};

	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;

	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling = {};

	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = NULL;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToCoverageEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState color_blend_attach_state = {};

	color_blend_attach_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attach_state.blendEnable = VK_FALSE;
	color_blend_attach_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attach_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attach_state.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attach_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attach_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attach_state.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo color_blending = {};

	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attach_state;

	color_blending.blendConstants[0] = 0.0f;
	color_blending.blendConstants[1] = 0.0f;
	color_blending.blendConstants[2] = 0.0f;
	color_blending.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipeline_layout_ci = {};

	pipeline_layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_ci.setLayoutCount = 0;
	pipeline_layout_ci.pSetLayouts = NULL;
	pipeline_layout_ci.pushConstantRangeCount = 0;
	pipeline_layout_ci.pPushConstantRanges = NULL;


	res = vkCreatePipelineLayout(ref->device, &pipeline_layout_ci, NULL, &ref->pipeline_layout);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to initialize fixed pipeline layout\n // Assertion: `vkCreatePipelineLayout != VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}


	VkGraphicsPipelineCreateInfo pipeline_ci = {};
	pipeline_ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_ci.stageCount = 2;
	pipeline_ci.pStages = shader_stages;

	pipeline_ci.pVertexInputState = &vert_input_info;
	pipeline_ci.pInputAssemblyState = &input_assembly;
	pipeline_ci.pViewportState = &viewport_state;
	pipeline_ci.pRasterizationState = &rasterizer;
	pipeline_ci.pMultisampleState = &multisampling;
	pipeline_ci.pDepthStencilState = NULL;
	pipeline_ci.pColorBlendState = &color_blending;
	pipeline_ci.pDynamicState = NULL;

	pipeline_ci.layout = ref->pipeline_layout;
	pipeline_ci.renderPass = ref->render_pass;
	pipeline_ci.subpass = 0;

	pipeline_ci.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_ci.basePipelineIndex = -1;

	res = vkCreateGraphicsPipelines(ref->device, VK_NULL_HANDLE, 1, &pipeline_ci, NULL, &ref->graphics_pipeline);
	if (res != VK_SUCCESS) {
		fprintf(stderr, "ERR: failed to initialize graphics pipeline\n // Assertion: `vkCreateGraphicsPipelines != VK_SUCCESS`\n");
		exit(EXIT_FAILURE);
	}

	vkDestroyShaderModule(ref->device, vert, NULL);
	vkDestroyShaderModule(ref->device, frag, NULL);
}

/**
 *	Create framebuffers to be used in the render pass.
 */
void create_framebuffers(struct _application *ref)
{
	array_init(&ref->swapc_framebuffers, sizeof(VkFramebuffer));
	array_resize(&ref->swapc_framebuffers, array_size(&ref->swapc_img_views), true);

	VkFramebuffer tmp[array_size(&ref->swapc_img_views)];

	for (int i = 0; i < array_size(&ref->swapc_img_views); i++) {

		VkImageView tmp0 = arr_get(ref->swapc_img_views, VkImageView, i);

		VkFramebufferCreateInfo framebuffer_ci = {};
		framebuffer_ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_ci.renderPass = ref->render_pass;
		framebuffer_ci.attachmentCount = 1;
		framebuffer_ci.pAttachments = &tmp0;
		framebuffer_ci.width = ref->swapc_extent.width;
		framebuffer_ci.height = ref->swapc_extent.height;
		framebuffer_ci.layers = 1;

		VkResult res = vkCreateFramebuffer(ref->device, &framebuffer_ci, NULL, &(((VkFramebuffer *) array_data(&ref->swapc_framebuffers))[i]));
		if (res != VK_SUCCESS) {
			fprintf(stderr, "ERR: failed to initialize framebuffer\n // Assertion: `vkCreateFramebuffer != VK_SUCCESS`\n");
			exit(EXIT_FAILURE);
		}
	}
}


/**
 *	Cleanup, free and destroy all used objects.
 */
void cleanup(struct _application *ref)
{
	cleanup_swapchain(ref);

	vkDestroyBuffer(ref->device, ref->vertex_buffer, NULL);
	vkFreeMemory(ref->device, ref->vertex_buffer_memory, NULL);

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

		VkSemaphore img = arr_get(ref->img_available_semaphore, VkSemaphore, i);
		VkSemaphore rendr = arr_get(ref->render_finished_semaphore, VkSemaphore, i);
		VkFence fence = arr_get(ref->in_flight_fences, VkFence, i);

		vkDestroySemaphore(ref->device, img, NULL);
		vkDestroySemaphore(ref->device, rendr, NULL);
		vkDestroyFence(ref->device, fence, NULL);
	}

	vkDestroyCommandPool(ref->device, ref->cmd_pool, NULL);

	vkDestroyDevice(ref->device, NULL);

	if (enable_validation_layers) {
		destroy_debug_utils_messenger_EXT(ref->vk_instance, ref->debug_messenger, NULL);
 	}
	
	vkDestroySurfaceKHR(ref->vk_instance, ref->surface, NULL);
	vkDestroyInstance(ref->vk_instance, NULL);

	array_free(&ref->queue_family_properties);
	array_free(&ref->instance_ext_names);
	array_free(&ref->device_ext_names);
	array_free(&ref->swapc_buffer);
	array_free(&ref->physical_devices);
	array_free(&ref->swapc_imgs);
	array_free(&ref->swapc_img_views);
	array_free(&ref->swapc_framebuffers);
	array_free(&ref->cmd_buffers);

	array_free(&ref->img_available_semaphore);
	array_free(&ref->render_finished_semaphore);
	array_free(&ref->in_flight_fences);
	array_free(&ref->imgs_in_flight);

	glfwDestroyWindow(ref->window);
	glfwTerminate();
}
