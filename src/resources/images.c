/**
 * images.c
 */

#include <IG_engine.h>
#include <IG_vkcore.h>
#include <IG_renderer.h>
#include <IG_memory.h>

#define BMP_IMPLEMENTATION
#include <bmp.h>

static void
IG_vk_transition_image_layout(VkImage image, VkImageLayout old, VkImageLayout new)
{
	VkCommandBuffer	cmd_buf = IG_vk_command_buffer_single();

	VkImageMemoryBarrier	barrier = 
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.oldLayout = old,
		.newLayout = new,
		.image = image,
        .subresourceRange		= (VkImageSubresourceRange)
		{
            .aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel	= 0,
            .levelCount		= 1,
            .baseArrayLayer	= 0,
            .layerCount		= 1
        }
	};

	VkPipelineStageFlags	src_flags;
	VkPipelineStageFlags	dst_flags;

	if (old == VK_IMAGE_LAYOUT_UNDEFINED && new == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		src_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (old == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		src_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dst_flags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
		IG_panic("invalid image layout transition.");

	vkCmdPipelineBarrier(cmd_buf, src_flags, dst_flags, 0, 0, NULL, 0, NULL, 1, &barrier);
	IG_vk_command_buffer_single_end(cmd_buf);
}

static void
IG_vk_copy_buffer_to_image(VkBuffer buffer, VkImage image, u32 width, u32 height)
{
	VkCommandBuffer	cmd_buf = IG_vk_command_buffer_single();

	VkBufferImageCopy img_copy = 
	{
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageOffset = (VkOffset3D) {0, 0, 0},
		.imageExtent = (VkExtent3D) {width, height, 1},
		.imageSubresource = (VkImageSubresourceLayers)
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};

	vkCmdCopyBufferToImage(cmd_buf, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &img_copy);
	IG_vk_command_buffer_single_end(cmd_buf);
}

void
IG_vk_image
(
	u32 width,
	u32 height,
	VkFormat format,
	VkImageTiling tiling,
	VkImageUsageFlags usage,
	VkMemoryPropertyFlags props,
	VkImage *image,
	VkDeviceMemory *image_mem
)
{
	const VkImageCreateInfo create_info = 
	{
		.sType			= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType		= VK_IMAGE_TYPE_2D,
		.samples		= VK_SAMPLE_COUNT_1_BIT,
		.flags			= 0,
		.format			= format,
		.tiling			= tiling,
		.usage			= usage,
		.extent			= (VkExtent3D)
		{
			.width = width,
			.height = height,
			.depth = 1,
		},
		.mipLevels		= 1,
		.arrayLayers	= 1,
		.sharingMode	= VK_SHARING_MODE_EXCLUSIVE,
	};

	if (vkCreateImage(IG.vulkan->device, &create_info, NULL, image) != VK_SUCCESS)
		IG_panic("failed to create VkImage.");

	VkMemoryRequirements mem_requirements;
	vkGetImageMemoryRequirements(IG.vulkan->device, *image, &mem_requirements);

	u32	mem_type_index = IG_vk_buffer_memory_type(mem_requirements.memoryTypeBits, props);

	const VkMemoryAllocateInfo alloc_info = 
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.memoryTypeIndex = mem_type_index,
		.allocationSize = mem_requirements.size,
	};

	if (vkAllocateMemory(IG.vulkan->device, &alloc_info, NULL, image_mem) != VK_SUCCESS)
		IG_panic("failed to allocate image memory.");

	vkBindImageMemory(IG.vulkan->device, *image, *image_mem, 0);
}

VkImageView
IG_vk_image_view(VkImage image, VkFormat format, VkImageAspectFlags flags)
{
	VkImageView	view;

	VkImageViewCreateInfo	create_info = 
	{
		.sType				= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    	.viewType			= VK_IMAGE_VIEW_TYPE_2D,
		.image				= image,
		.format				= format,
		.subresourceRange	= (VkImageSubresourceRange)
		{
			.aspectMask		= flags,
			.baseMipLevel	= 0,
			.levelCount		= 1,
			.baseArrayLayer = 0,
			.layerCount		= 1,
		},
	};
	vkCreateImageView(IG.vulkan->device, &create_info, NULL, &view);
	return (view);
}

void
IG_vk_texture_image(void)
{
	BMPfile		image = {0};
	uint32_t	width;
	uint32_t	height;
	uint32_t	channels;

	if (!bmp_open("viking_room.bmp", &image))
		IG_panic("failed to open texture image.");
	
	uint32_t		*colors = bmp_parse(&image, &width, &height, &channels);

	info("bmp - %d * %d", width, height);

	VkDeviceSize	img_size = 4 * width * height;
	VkBuffer		staging_buf;
	VkDeviceMemory	staging_mem;
	void			*staging_data;

	IG_vk_buffer
	(
		img_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		&staging_buf, &staging_mem,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	vkMapMemory(IG.vulkan->device, staging_mem, 0, img_size, 0, &staging_data);
	memcpy(staging_data, colors, img_size);
	vkUnmapMemory(IG.vulkan->device, staging_mem);
	free(colors);

	IG_vk_image
	(
		width, height, VK_FORMAT_B8G8R8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&IG.buffer->texture, &IG.buffer->texture_mem
	);

	IG_vk_transition_image_layout
	(
		IG.buffer->texture,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);
	IG_vk_copy_buffer_to_image(staging_buf, IG.buffer->texture, width, height);
	IG_vk_transition_image_layout
	(
		IG.buffer->texture,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);

	vkFreeMemory(IG.vulkan->device, staging_mem, NULL);
	vkDestroyBuffer(IG.vulkan->device, staging_buf, NULL);
}

void
IG_vk_texture_image_view(void)
{
	IG.buffer->texture_view = IG_vk_image_view(IG.buffer->texture, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void
IG_vk_texture_sampler(void)
{
	VkPhysicalDeviceProperties	device_properties;

	vkGetPhysicalDeviceProperties(IG.vulkan->physical_device, &device_properties);

	const VkSamplerCreateInfo create_info =
	{
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.minFilter = VK_FILTER_NEAREST,
		.magFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.anisotropyEnable = VK_FALSE,
		.maxAnisotropy = device_properties.limits.maxSamplerAnisotropy,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.compareEnable = VK_FALSE,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE,
	};
	
	if (vkCreateSampler(IG.vulkan->device, &create_info, NULL, &IG.buffer->texture_sampler) != VK_SUCCESS)
		IG_panic("failed to create texture sampler.");
}

arr_decl(VkFormat, VkFormats);

static VkFormat
IG_vk_supported_depth_format(VkFormats formats, VkImageTiling tiling, VkFormatFeatureFlags flags)
{
	VkFormat	*it;

	arr_foreach(VkFormat, format, formats)
	{
		VkFormatProperties	props;
		
		it = format;
		vkGetPhysicalDeviceFormatProperties(IG.vulkan->physical_device, *format, &props);

		if ((tiling == VK_IMAGE_TILING_LINEAR) && ((props.linearTilingFeatures & flags) == flags))
			break ;
		if ((tiling == VK_IMAGE_TILING_OPTIMAL) && ((props.optimalTilingFeatures & flags) == flags))
			break ;
	}
	if (arr_index(formats, it) >= arr_count(formats))
		IG_panic("failed to find a supported depth format.");
	return (*it);
}

#define	FORMAT_COUNT	3

VkFormat
IG_vk_depth_format()
{
	VkFormat	format_list[FORMAT_COUNT] = 
	{
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
	};
	VkFormats	formats = 
	{
		.count = FORMAT_COUNT,
		.capacity = FORMAT_COUNT,
		.items = format_list,
	};
	return (IG_vk_supported_depth_format
	(
		formats, VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	));
}

void
IG_vk_depth_resources(void)
{
	VkFormat	depth_format = IG_vk_depth_format();

	IG_vk_image
	(
		IG.renderer->swap_extent.width, IG.renderer->swap_extent.height,
		depth_format, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&IG.buffer->depth, &IG.buffer->depth_mem
	);
	IG.buffer->depth_view = IG_vk_image_view(IG.buffer->depth, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);
}

