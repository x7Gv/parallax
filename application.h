#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <vulkan/vulkan.h>

typedef struct _application application;

struct _application
{
	VkInstance vk_instance;
};

void run(struct _application *ref);

void main_loop(struct _application *ref);

void init_vk(struct _application *ref);

void cleanup(struct _application *ref);

#endif