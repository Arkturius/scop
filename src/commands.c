/**
 * commands.c
 */

#include <scop.h>
#include <stdint.h>

void
app_vk_command_pool(void)
{
	App	*app = App_getinstance();

	const VkCommandPoolCreateInfo create_info = 
	{
		.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags				= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex	= 0,
	};

	if (vkCreateCommandPool(app->device, &create_info, NULL, &app->cmd_pool) != VK_SUCCESS)
		app_panic("failed to create command pool.");

	app->state = VK_CMD_POOL;
}

void
app_vk_command_buffers(void)
{
	App *app = App_getinstance();

	const VkCommandBufferAllocateInfo alloc_info = 
	{
		.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool		= app->cmd_pool,
		.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount	= MAX_FRAMES,
	};

	vec_count(app->cmd_buffers) = 0;
	if (!app->cmd_buffers.items)
		vec_reserve(app->cmd_buffers, MAX_FRAMES);
	if (vkAllocateCommandBuffers(app->device, &alloc_info, app->cmd_buffers.items) != VK_SUCCESS)
		app_panic("failed to allocate command buffers.");
	vec_count(app->cmd_buffers) = MAX_FRAMES;

	app->state = VK_CMD_BUFFER;
}

static void 
app_vk_transition_image_layout
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
	App	*app = App_getinstance();

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
        .image					= app->swap_images.items[image_index],
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

	vkCmdPipelineBarrier2(app->cmd_buffers.items[app->current_frame], &dep_info);
}

void
app_vk_record_cmd_buffer(u32 image_index)
{
	App				*app = App_getinstance();

	VkCommandBuffer	cmd_buffer = app->cmd_buffers.items[app->current_frame];

	const VkCommandBufferBeginInfo begin_info = 
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};

	vkBeginCommandBuffer(cmd_buffer, &begin_info);
	app_vk_transition_image_layout
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
		.renderArea				= { .offset = { 0, 0 }, .extent = app->swap_extent },
    	.layerCount				= 1,
    	.colorAttachmentCount	= 1,
    	.pColorAttachments		= &(VkRenderingAttachmentInfo)
		{
			.sType			= VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageView		= app->swap_views.items[image_index],
			.imageLayout	= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp		= VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue		= clear_color
		},
	};

	vkCmdBeginRendering(cmd_buffer, &rendering_info);
	vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, app->pipeline);
	vkCmdSetViewport
	(
		cmd_buffer, 0, 1, &(VkViewport)
		{
			.x			= 0.0f,
			.y			= 0.0f,
			.width		= app->swap_extent.width,
			.height		= app->swap_extent.height,
			.minDepth	= 0.0f,
			.maxDepth	= 0.0f,
		}
	);
	vkCmdSetScissor
	(
		cmd_buffer, 0, 1, &(VkRect2D)
		{
			.offset		= (VkOffset2D){0, 0},
			.extent 	= app->swap_extent,
		}
	);
	vkCmdDraw(cmd_buffer, 3, 1, 0, 0);
	vkCmdEndRendering(cmd_buffer);

	app_vk_transition_image_layout
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
app_vk_draw_frame(void)
{
	App	*app = App_getinstance();
	u32	image_index;

	VkCommandBuffer	cmd_buffer			= app->cmd_buffers.items[app->current_frame];
	VkSemaphore		present_semaphore	= app->present_semaphores.items[app->current_frame];
	VkFence			draw_fence			= app->draw_fences.items[app->current_frame];
	VkResult		result;

	while (vkWaitForFences(app->device, 1, &draw_fence, VK_TRUE, UINT64_MAX) == VK_TIMEOUT)
		;

	result = vkAcquireNextImageKHR
	(
		app->device, app->swapchain, UINT64_MAX,
		present_semaphore, NULL, &image_index
	);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		vkResetFences(app->device, 1, &draw_fence);
		app_vk_swapchain_recreate();
		return ;
	}
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		app_panic("failed to acquire next image.");
	
	VkSemaphore		render_semaphore	= app->render_semaphores.items[image_index];

	app_vk_record_cmd_buffer(image_index);
	vkResetFences(app->device, 1, &draw_fence);

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
	vkQueueSubmit(app->graphics_queue, 1, &sub_info, draw_fence);

	const VkPresentInfoKHR pres_info =
	{
		.sType				= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount	= 1,
		.swapchainCount		= 1,
		.pImageIndices		= &image_index,
		.pWaitSemaphores	= &render_semaphore,
		.pSwapchains		= &app->swapchain,
	};

	result = vkQueuePresentKHR(app->graphics_queue, &pres_info);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || app->fb_resized)
	{
		app->fb_resized = false;
		app_vk_swapchain_recreate();
	}
	else if (result != VK_SUCCESS)
		app_panic("failed to present swap chain image.");

	app->current_frame = (app->current_frame + 1) % MAX_FRAMES;
}
