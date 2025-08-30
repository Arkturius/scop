/**
 * pipeline.c
 */

#include <assert.h>

#include <scop.h>
#include <vulkan/vulkan_core.h>

#define	APP_SHADER_PATH	"src/shaders/shader.spv"

void
app_vk_pipeline(void)
{
	App				*app = App_getinstance();
	ShaderFile		shader_file;
	VkShaderModule	shader;

	shader_file = app_shader_read(APP_SHADER_PATH);
	if (!shader_file.content)
		app_panic("failed to load shader.");

	assert(((uptr)shader_file.content & (sizeof(uint32_t) - 1)) == 0);

	const VkShaderModuleCreateInfo	create_info = 
	{
		.sType		= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize	= shader_file.size,
		.pCode		= (uint32_t *)shader_file.content,
	};

	if (vkCreateShaderModule(app->logical_device, &create_info, NULL, &shader) != VK_SUCCESS)
	{
		app_shader_cleanup(shader_file);
		app_panic("failed to create shader module.");
	}
	app_shader_cleanup(shader_file);

	const VkPipelineShaderStageCreateInfo	vert_info = 
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = shader,
		.pName = "vert_main",
	};
	const VkPipelineShaderStageCreateInfo	frag_info = 
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = shader,
		.pName = "frag_main",
	};
	const VkPipelineShaderStageCreateInfo	shader_stages[2] = {vert_info, frag_info};

	vkDestroyShaderModule(app->logical_device, shader, NULL);
	app->state = VK_PIPELINE;
}
