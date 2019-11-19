#ifndef _SWAPC_H_
#define _SWAPC_H_

#include "application.h"

VkSurfaceFormatKHR choose_swp_surf_format(array available_formats);

VkPresentModeKHR choose_swp_present_mode(array present_modes);

VkExtent2D choose_swp_extent(VkSurfaceCapabilitiesKHR capabilities, GLFWwindow *window);

swapchain_supp_detail_t query_swapchain_supp(VkPhysicalDevice device, VkSurfaceKHR surface);

void init_swapchain(struct _application *ref);

void cleanup_swapchain(struct _application *ref);

void recreate_swapchain(struct _application *ref);

#endif