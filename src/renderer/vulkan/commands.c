/**
 * commands.c
 */

#include <IG_engine.h>
#include <IG_vkcore.h>
#include <IG_renderer.h>
#include <IG_memory.h>

void
IG_vk_command_pool(void)
{
	const VkCommandPoolCreateInfo create_info = 
	{
		.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags				= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex	= 0,
	};

	if (vkCreateCommandPool(IG.vulkan->device, &create_info, NULL, &IG.renderer->cmd_pool) != VK_SUCCESS)
		IG_panic("failed to create command pool.");

	IG.state = IG_CMD_POOL;
}

void
IG_vk_command_buffers(void)
{
	const VkCommandBufferAllocateInfo alloc_info = 
	{
		.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool		= IG.renderer->cmd_pool,
		.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount	= MAX_FRAMES,
	};

	arr_count(IG.renderer->cmd_buffers) = 0;
	if (!IG.renderer->cmd_buffers.items)
		arr_reserve(IG.renderer->cmd_buffers, MAX_FRAMES);
	if (vkAllocateCommandBuffers(IG.vulkan->device, &alloc_info, IG.renderer->cmd_buffers.items) != VK_SUCCESS)
		IG_panic("failed to allocate command buffers.");
	arr_count(IG.renderer->cmd_buffers) = MAX_FRAMES;

	IG.state = IG_CMD_BUFFER;
}

static void 
IG_vk_transition_image_layout
(
    u32						image_index,
    VkImageLayout			old_layout,
    VkImageLayout			new_layout,
    VkAccessFlags2			src_access_mask,
    VkAccessFlags2			dst_access_mask,
    VkPipelineStageFlags2	src_stage_mask,
    VkPipelineStageFlags2	dst_stage_mask
)
{
	const VkImageMemoryBarrier2 barrier = 
	{
		.sType					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.oldLayout				= old_layout,
		.newLayout				= new_layout,
        .srcStageMask			= src_stage_mask,
        .srcAccessMask			= src_access_mask,
        .dstStageMask			= dst_stage_mask,
        .dstAccessMask			= dst_access_mask,
        .srcQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED,
        .image					= IG.renderer->swap_images.items[image_index],
        .subresourceRange		= (VkImageSubresourceRange)
		{
            .aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel	= 0,
            .levelCount		= 1,
            .baseArrayLayer	= 0,
            .layerCount		= 1
        }
	};

	const VkDependencyInfo dep_info =
	{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier,
	};

	vkCmdPipelineBarrier2(IG.renderer->cmd_buffers.items[IG.current_frame], &dep_info);
}

void
IG_vk_record_cmd_buffer(u32 image_index)
{
	VkCommandBuffer	cmd_buffer = IG.renderer->cmd_buffers.items[IG.current_frame];

	const VkCommandBufferBeginInfo begin_info = 
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};

	vkBeginCommandBuffer(cmd_buffer, &begin_info);
	IG_vk_transition_image_layout
	(
		image_index,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		0,
		VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
		VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
	);

	VkClearValue	clear_color		= (VkClearValue){(VkClearColorValue){{0, 0, 0, 1}}};
	VkRenderingInfo	rendering_info	=
	{
		.sType					= VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea				= { .offset = { 0, 0 }, .extent = IG.renderer->swap_extent },
    	.layerCount				= 1,
    	.colorAttachmentCount	= 1,
    	.pColorAttachments		= &(VkRenderingAttachmentInfo)
		{
			.sType			= VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageView		= IG.renderer->swap_views.items[image_index],
			.imageLayout	= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp		= VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue		= clear_color
		},
	};

	vkCmdBeginRendering(cmd_buffer, &rendering_info);
	vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, IG.renderer->pipeline);
	vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &IG.buffer->vertex, &(const VkDeviceSize){0});
	vkCmdBindIndexBuffer(cmd_buffer, IG.buffer->index, 0, VK_INDEX_TYPE_UINT32);
	vkCmdSetViewport
	(
		cmd_buffer, 0, 1, &(VkViewport)
		{
			.x			= 0.0f,
			.y			= 0.0f,
			.width		= IG.renderer->swap_extent.width,
			.height		= IG.renderer->swap_extent.height,
			.minDepth	= 0.0f,
			.maxDepth	= 0.0f,
		}
	);
	vkCmdSetScissor
	(
		cmd_buffer, 0, 1, &(VkRect2D)
		{
			.offset		= (VkOffset2D){0, 0},
			.extent 	= IG.renderer->swap_extent,
		}
	);
	vkCmdDrawIndexed(cmd_buffer, array_len(indices), 1, 0, 0, 0);
	vkCmdEndRendering(cmd_buffer);

	IG_vk_transition_image_layout
	(
		image_index,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
		0,
		VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT
	);
	vkEndCommandBuffer(cmd_buffer);
}

void
IG_vk_draw_frame(void)
{
	u32	image_index;

	VkCommandBuffer	cmd_buffer			= IG.renderer->cmd_buffers.items[IG.current_frame];
	VkSemaphore		present_semaphore	= IG.renderer->present_semaphores.items[IG.current_frame];
	VkFence			draw_fence			= IG.renderer->draw_fences.items[IG.current_frame];
	VkResult		result;

	while (vkWaitForFences(IG.vulkan->device, 1, &draw_fence, VK_TRUE, UINT64_MAX) == VK_TIMEOUT);

	result = vkAcquireNextImageKHR
	(
		IG.vulkan->device, IG.renderer->swapchain, UINT64_MAX,
		present_semaphore, NULL, &image_index
	);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		vkResetFences(IG.vulkan->device, 1, &draw_fence);
		IG_vk_swapchain_recreate();
		return ;
	}
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		IG_panic("failed to acquire next image.");
	
	VkSemaphore		render_semaphore	= IG.renderer->render_semaphores.items[image_index];

	IG_vk_record_cmd_buffer(image_index);
	vkResetFences(IG.vulkan->device, 1, &draw_fence);

	const VkSubmitInfo sub_info = 
	{
		.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pWaitDstStageMask		= &(VkPipelineStageFlags){VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
		.commandBufferCount		= 1,
		.waitSemaphoreCount		= 1,
		.signalSemaphoreCount	= 1,
		.pCommandBuffers		= &cmd_buffer,
		.pWaitSemaphores		= &present_semaphore,
		.pSignalSemaphores		= &render_semaphore,
	};
	vkQueueSubmit(IG.vulkan->graphics_queue, 1, &sub_info, draw_fence);

	const VkPresentInfoKHR pres_info =
	{
		.sType				= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount	= 1,
		.swapchainCount		= 1,
		.pImageIndices		= &image_index,
		.pWaitSemaphores	= &render_semaphore,
		.pSwapchains		= &IG.renderer->swapchain,
	};

	result = vkQueuePresentKHR(IG.vulkan->graphics_queue, &pres_info);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || IG.fb_resized)
	{
		IG.fb_resized = false;
		IG_vk_swapchain_recreate();
	}
	else if (result != VK_SUCCESS)
		IG_panic("failed to present swap chain image.");

	IG.current_frame = (IG.current_frame + 1) % MAX_FRAMES;
}
