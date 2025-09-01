/**
 * sync.c
 */

#include <scop.h>

static void
app_vk_sync_draw_fences()
{
	App	*app = App_getinstance();

	const VkFenceCreateInfo		fen_info =
	{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	for (u32 frame = 0; frame < MAX_FRAMES; ++frame)
	{
		VkFence		fen;

		if (vkCreateFence(app->device, &fen_info, NULL, &fen) != VK_SUCCESS)
			app_panic("failed to create 'draw' fence.");
		vec_append(app->draw_fences, fen);
	}
}

static void
app_vk_sync_present_semaphores()
{
	App	*app = App_getinstance();

	const VkSemaphoreCreateInfo	sem_info = 
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
	};
	
	for (u32 frame = 0; frame < MAX_FRAMES; ++frame)
	{
		VkSemaphore	sem;

		if (vkCreateSemaphore(app->device, &sem_info, NULL, &sem) != VK_SUCCESS)
			app_panic("failed to create 'present' semaphore.");
		vec_append(app->present_semaphores, sem);
	}

}

void
app_vk_sync_render_semaphores()
{
	App	*app = App_getinstance();

	const VkSemaphoreCreateInfo	sem_info =
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	vec_foreach(VkImage, image, app->swap_images)
	{
		VkSemaphore	sem;

		if (vkCreateSemaphore(app->device, &sem_info, NULL, &sem) != VK_SUCCESS)
			app_panic("failed to create 'render' semaphore.");
		vec_append(app->render_semaphores, sem);
	}
}

void
app_vk_sync_objects(void)
{
	App	*app = App_getinstance();
	
	app_vk_sync_render_semaphores();
	app_vk_sync_present_semaphores();
	app_vk_sync_draw_fences();

	app->state = VK_SYNC;
}
