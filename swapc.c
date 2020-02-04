#include "swapc.h"

VkSurfaceFormatKHR choose_swp_surf_format(array available_formats)
{
	for (uint32_t i = 0; i < array_size(&available_formats); i++) {

		VkSurfaceFormatKHR form = arr_get(available_formats, VkSurfaceFormatKHR, i);
		if (form.format == VK_FORMAT_B8G8R8A8_UNORM && form.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return form;
		}
	}

	return arr_get(available_formats, VkSurfaceFormatKHR, 0);
}



VkPresentModeKHR choose_swp_present_mode(array present_modes)
{
	for (uint32_t i = 0; i < array_size(&present_modes); i++) {

		VkPresentModeKHR present = arr_get(present_modes, VkPresentModeKHR, i);
		if (present == VK_PRESENT_MODE_MAILBOX_KHR) {
			return present;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}



VkExtent2D choose_swp_extent(VkSurfaceCapabilitiesKHR capabilities, GLFWwindow *window)
{

	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actual_extent = {width, height};

		if (actual_extent.width < capabilities.minImageExtent.width) {
			actual_extent.width = capabilities.minImageExtent.width;
		}
		else if (actual_extent.width > capabilities.maxImageExtent.width) {
			actual_extent.width = capabilities.maxImageExtent.width;
		}

		if (actual_extent.height < capabilities.minImageExtent.height) {
			actual_extent.height = capabilities.minImageExtent.height;
		}
		else if (actual_extent.height > capabilities.maxImageExtent.height) {
			actual_extent.height = capabilities.maxImageExtent.height;
		}

		return actual_extent;
	}
}

swapchain_supp_detail_t query_swapchain_supp(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	swapchain_supp_detail_t details = {};

	array_init(&details.formats, sizeof(VkSurfaceFormatKHR));
	array_init(&details.present_modes, sizeof(VkPresentModeKHR));

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, NULL);

	VkSurfaceFormatKHR tmp[format_count];

	if (format_count != 0) {
		array_resize(&details.formats, format_count, true);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, (VkSurfaceFormatKHR *) array_data(&details.formats));
	}

	uint32_t present_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, NULL);

	VkPresentModeKHR tmp0[present_mode_count];

	if (present_mode_count != 0) {
		array_resize(&details.present_modes, present_mode_count, true);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, (VkPresentModeKHR *) array_data(&details.present_modes));
	}

	return details;
}

void init_swapchain(struct _application *ref)
{
	swapchain_supp_detail_t swapchain_support = query_swapchain_supp(PHYSDEV(0), ref->surface);

	VkSurfaceFormatKHR surface_format = choose_swp_surf_format(swapchain_support.formats);
	VkPresentModeKHR present_mode = choose_swp_present_mode(swapchain_support.present_modes);
	VkExtent2D extent = choose_swp_extent(swapchain_support.capabilities, ref->window);

	uint32_t img_count = swapchain_support.capabilities.minImageCount + 1;

	if (swapchain_support.capabilities.maxImageCount > 0 && img_count > swapchain_support.capabilities.maxImageCount) {
		img_count = swapchain_support.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swp_ci = {};

	swp_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swp_ci.surface = ref->surface;
	swp_ci.minImageCount = img_count;
	swp_ci.imageFormat = surface_format.format;
	swp_ci.imageColorSpace = surface_format.colorSpace;
	swp_ci.imageExtent = extent;
	swp_ci.imageArrayLayers = 1;
	swp_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	queue_family_indices_t indices = query_queue_families(PHYSDEV(0), ref->surface);
	uint32_t queue_family_indices[] = {indices.graphics_family, indices.present_family};

	if (indices.graphics_family != indices.present_family) {

		swp_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swp_ci.queueFamilyIndexCount = 2;
		swp_ci.pQueueFamilyIndices = queue_family_indices;
	}
	else {
		swp_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	swp_ci.preTransform = swapchain_support.capabilities.currentTransform;
	swp_ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swp_ci.presentMode = present_mode;
	swp_ci.clipped = VK_TRUE;

	swp_ci.oldSwapchain = VK_NULL_HANDLE;

	VkResult res = vkCreateSwapchainKHR(ref->device, &swp_ci, NULL, &ref->swapchain);
		if (res != VK_SUCCESS) {
			fprintf(stderr, "ERR: failed to initialize Swapchain\n // Assertion: `vkCreateSwapchainKHR != VK_SUCCESS`\n");
			exit(EXIT_FAILURE);
		}

	vkGetSwapchainImagesKHR(ref->device, ref->swapchain, &img_count, NULL);

	array_init(&ref->swapc_imgs, sizeof(VkImage));
	array_resize(&ref->swapc_imgs, img_count, true);

	VkImage tmp[img_count];
	vkGetSwapchainImagesKHR(ref->device, ref->swapchain, &img_count, (VkImage *) array_data(&ref->swapc_imgs));

	ref->swapc_img_format = surface_format.format;
	ref->swapc_extent = extent;
}

void cleanup_swapchain(struct _application *ref)
{
	for (int i = 0; i < array_size(&ref->swapc_framebuffers); i++) {

		VkFramebuffer framebuffer = arr_get(ref->swapc_framebuffers, VkFramebuffer, i);
		vkDestroyFramebuffer(ref->device, framebuffer, NULL);
	}

	vkDestroyPipeline(ref->device, ref->graphics_pipeline, NULL);
	vkDestroyPipelineLayout(ref->device, ref->pipeline_layout, NULL);
	vkDestroyRenderPass(ref->device, ref->render_pass, NULL);

	for (int i = 0; i < array_size(&ref->swapc_img_views); i++) {

		VkImageView img_view = arr_get(ref->swapc_img_views, VkImageView, i);
		vkDestroyImageView(ref->device, img_view, NULL);
	}

	vkDestroySwapchainKHR(ref->device, ref->swapchain, NULL);

	for (size_t i = 0; i < array_size(&ref->swapc_imgs); i++) {
		vkDestroyBuffer(ref->device, ((VkBuffer *) array_data(&ref->uniform_buffers))[i], NULL);
		vkFreeMemory(ref->device, ((VkDeviceMemory *) array_data(&ref->uniform_buffers_memory))[i], NULL);
	}

	vkDestroyDescriptorPool(ref->device, ref->descriptor_pool, NULL);
}

void recreate_swapchain(struct _application *ref)
{

	int width = 0, height = 9;
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(ref->window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(ref->device);

	cleanup_swapchain(ref);

	init_swapchain(ref);
	init_image_views(ref);
	create_renderpass(ref);
	create_graphics_pipeline(ref);
	create_framebuffers(ref);

	create_uniform_buffers(ref);
	create_descriptor_pool(ref);
	create_descriptor_sets(ref);
	create_command_buffers(ref);
}