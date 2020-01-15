#include "validations.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	void *pUserData) 
{
	fprintf(stderr, "vL : %s\n", pCallbackData->pMessage);
	return VK_FALSE;
}

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

void populate_debug_messenger_ci(VkDebugUtilsMessengerCreateInfoEXT *create_info) 
{
	create_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info->messageSeverity = 
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info->messageType = 
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info->pfnUserCallback = debugCallback;
}

int check_validation_layer_support()
{
	uint32_t layer_count;
	vkEnumerateInstanceLayerProperties(&layer_count, NULL);

	array available_layers;
	array_init(&available_layers, sizeof(VkLayerProperties));
	array_resize(&available_layers, layer_count, true);

	vkEnumerateInstanceLayerProperties(&layer_count, (VkLayerProperties *) array_data(&available_layers));

	return 0;
}


