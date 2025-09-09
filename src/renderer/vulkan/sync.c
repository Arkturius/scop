/**
 * sync.c
 */

#include <IG_engine.h>
#include <IG_vkcore.h>
#include <IG_renderer.h>

static void
IG_vk_sync_draw_fences()
{
	const VkFenceCreateInfo		fen_info =
	{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	for (u32 frame = 0; frame < MAX_FRAMES; ++frame)
	{
		VkFence		fen;

		if (vkCreateFence(IG.vulkan->device, &fen_info, NULL, &fen) != VK_SUCCESS)
			IG_panic("failed to create 'draw' fence.");
		arr_append(IG.renderer->draw_fences, fen);
	}
}

static void
IG_vk_sync_present_semaphores()
{
	const VkSemaphoreCreateInfo	sem_info = 
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
	};
	
	for (u32 frame = 0; frame < MAX_FRAMES; ++frame)
	{
		VkSemaphore	sem;

		if (vkCreateSemaphore(IG.vulkan->device, &sem_info, NULL, &sem) != VK_SUCCESS)
			IG_panic("failed to create 'present' semaphore.");
		arr_append(IG.renderer->present_semaphores, sem);
	}

}

void
IG_vk_sync_render_semaphores()
{
	const VkSemaphoreCreateInfo	sem_info =
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	arr_foreach(VkImage, image, IG.renderer->swap_images)
	{
		VkSemaphore	sem;

		if (vkCreateSemaphore(IG.vulkan->device, &sem_info, NULL, &sem) != VK_SUCCESS)
			IG_panic("failed to create 'render' semaphore.");
		arr_append(IG.renderer->render_semaphores, sem);
	}
}

void
IG_vk_sync_objects(void)
{
	IG_vk_sync_render_semaphores();
	IG_vk_sync_present_semaphores();
	IG_vk_sync_draw_fences();

	IG.state = IG_SYNC;
}
