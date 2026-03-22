/**
 * descriptors.c
 */

#include <math.h>
#include <time.h>

#include <IG_engine.h>
#include <IG_vkcore.h>
#include <IG_renderer.h>
#include <IG_memory.h>
#include <vulkan/vulkan_core.h>

#define	DESCRIPTOR_COUNT	2

void
IG_vk_descriptor_set_layout(void)
{
	const VkDescriptorSetLayoutBinding	dslb[DESCRIPTOR_COUNT] = 
	{
		{
			.binding			= 0,
			.descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount	= 1,
			.stageFlags			= VK_SHADER_STAGE_VERTEX_BIT,
		},
		{
			.binding			= 1,
			.descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount	= 1,
			.stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT,
		},
	};

	const VkDescriptorSetLayoutCreateInfo	create_info = 
	{
		.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount	= 2,
		.pBindings		= dslb,
	};

	if (vkCreateDescriptorSetLayout(IG.vulkan->device, &create_info, NULL, &IG.renderer->ds_layout) != VK_SUCCESS)
		IG_panic("failed to create DescriptorSetLayout.");
}

void
IG_vk_uniform_buffers(void)
{
	arr_count(IG.buffer->uniform) = 0;
	arr_count(IG.buffer->uniform_mem) = 0;
	arr_count(IG.buffer->uniform_mapped) = 0;
	
	for (u32 i = 0; i < MAX_FRAMES; ++i)
	{
		VkDeviceSize	size = sizeof(UBO);
		VkBuffer		buf;
		VkDeviceMemory	mem;
		void			*data;

		IG_vk_buffer
		(
			size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			&buf, &mem,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
		vkMapMemory(IG.vulkan->device, mem, 0, size, 0, &data);
		arr_append(IG.buffer->uniform, buf);
		arr_append(IG.buffer->uniform_mem, mem);
		arr_append(IG.buffer->uniform_mapped, data);
	}

	IG.state = IG_UNIFORM_BUFFER;
}

static double
IG_clock(void)
{
	static struct timespec	ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1000000000 + ts.tv_nsec);
}

void
IG_vk_update_uniforms(void)
{
	static double	start = 0.0f;

	if (start == 0.0f)
		start = IG_clock();

	double	now = IG_clock();
	double	time = (now - start) / 1000000000;

//	uniform_object.model = mat4_rotate((M_PI_2), vec3(0, 0, 1));
	IG.renderer->uniform_object.model = mat4_identity();
	
	Vec3	target = IG.renderer->pos;

	target.y += 1;

	IG.renderer->uniform_object.view = mat4_lookat(IG.renderer->pos, target, vec3(0, 0, 1));
	IG.renderer->uniform_object.proj = mat4_perspective(M_PI_4, (f32)IG.renderer->swap_extent.width / (f32)IG.renderer->swap_extent.height, 0.01f, 1000.0f);

	M4(IG.renderer->uniform_object.proj, 1, 1) = -M4(IG.renderer->uniform_object.proj, 1, 1); 

	memcpy(IG.buffer->uniform_mapped.items[IG.current_frame], &IG.renderer->uniform_object, sizeof(UBO));
}

void
IG_vk_descriptor_pool()
{
	const VkDescriptorPoolSize	size[DESCRIPTOR_COUNT] = 
	{
		{
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = MAX_FRAMES,
		},
		{
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = MAX_FRAMES,
		},
	};

	const VkDescriptorPoolCreateInfo	create_info = 
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		.maxSets = MAX_FRAMES,
		.poolSizeCount = 2,
		.pPoolSizes = size,
	};

	if (vkCreateDescriptorPool(IG.vulkan->device, &create_info, NULL, &IG.renderer->descriptor_pool) != VK_SUCCESS)
		IG_panic("failed to create Descriptor Pool.");

	IG.state = IG_DESCRIPTOR_POOL;
}

arr_decl(VkDescriptorSetLayout, VkDescriptorSetLayouts);

void
IG_vk_descriptor_sets(void)
{
	VkDescriptorSetLayouts	layouts = {0};

	arr_reserve(layouts, MAX_FRAMES);
	arr_reserve(IG.renderer->descriptor_sets, MAX_FRAMES);

	for (u32 i = 0; i < MAX_FRAMES; ++i)
		arr_append(layouts, IG.renderer->ds_layout);

	VkDescriptorSetAllocateInfo	alloc_info = 
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = IG.renderer->descriptor_pool,
		.descriptorSetCount = MAX_FRAMES,
		.pSetLayouts = layouts.items,
	};

	vkAllocateDescriptorSets(IG.vulkan->device, &alloc_info, IG.renderer->descriptor_sets.items);
	arr_count(IG.renderer->descriptor_sets) = MAX_FRAMES;

	for (size_t i = 0; i < MAX_FRAMES; i++)
	{
		VkDescriptorBufferInfo	buffer_info = 
		{
			.buffer = IG.buffer->uniform.items[i],
			.offset = 0,
			.range = sizeof(UBO),
		};
		VkDescriptorImageInfo	image_info =
		{
			.sampler = IG.buffer->texture_sampler,
			.imageView = IG.buffer->texture_view,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		};
		VkWriteDescriptorSet	write_info[DESCRIPTOR_COUNT] =
		{
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = IG.renderer->descriptor_sets.items[i],
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.pBufferInfo = &buffer_info,
			},
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = IG.renderer->descriptor_sets.items[i],
				.dstBinding = 1,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo = &image_info,
			},
		};
		vkUpdateDescriptorSets(IG.vulkan->device, 2, write_info, 0, NULL);
	}

	IG.state = IG_DESCRIPTOR_SETS;
}
