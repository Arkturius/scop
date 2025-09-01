/**
 * pipeline.c
 */

#include <assert.h>

#include <scop.h>
#include <vulkan/vulkan_core.h>

#define	APP_SHADER_PATH	"src/shaders/shader.spv"

static VkShaderModule 
app_vk_shader_module(void)
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

	VkResult	ret = vkCreateShaderModule(app->device, &create_info, NULL, &shader);
	
	app_shader_cleanup(shader_file);
	if (ret != VK_SUCCESS)
		app_panic("failed to create shader module.");

	return (shader);
}

void
app_vk_pipeline(void)
{
	App				*app = App_getinstance();
	VkShaderModule	shader_module;

	shader_module = app_vk_shader_module();

	const VkPipelineShaderStageCreateInfo shader_stages[2] = 
	{
		{
			.sType	= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage	= VK_SHADER_STAGE_VERTEX_BIT,
			.module = shader_module,
			.pName	= "vert_main",
		},
		{
			.sType	= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage	= VK_SHADER_STAGE_FRAGMENT_BIT,
			.module	= shader_module,
			.pName	= "frag_main",
		}
	};

	VkVertexInputBindingDescription	vib_desc = app_vk_vertex_get_binding();

	const VkPipelineVertexInputStateCreateInfo vis_info = 
	{
		.sType		= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.vertexAttributeDescriptionCount = 2,
		.pVertexBindingDescriptions = &vib_desc,
		.pVertexAttributeDescriptions = app_vk_vertex_get_attributes(),
	};

	const VkPipelineInputAssemblyStateCreateInfo ias_info =
	{
		.sType		= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology	= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	};

	const VkPipelineViewportStateCreateInfo vs_info = 
	{
		.sType			= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount	= 1,
		.scissorCount	= 1,
		.pViewports		= &(VkViewport)
		{
			.x			= 0.0f,
			.y			= 0.0f,
			.width		= app->swap_extent.width,
			.height		= app->swap_extent.height,
			.minDepth	= 0.0f,
			.maxDepth	= 0.0f,
		},
		.pScissors		= &(VkRect2D)
		{
			.offset		= (VkOffset2D){0, 0},
			.extent 	= app->swap_extent,
		},
	};

	const VkPipelineRasterizationStateCreateInfo rast_info = 
	{	
		.sType					= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.polygonMode			= VK_POLYGON_MODE_FILL,
		.cullMode				= VK_CULL_MODE_BACK_BIT,
		.frontFace				= VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable		= VK_FALSE,
		.depthBiasSlopeFactor	= 1.0f,
		.lineWidth				= 1.0f,
	};

	const VkPipelineMultisampleStateCreateInfo mss_info =
	{
		.sType					= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples	= VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable	= VK_FALSE,
	};
	
	const VkPipelineColorBlendStateCreateInfo cbs_info = 
	{
		.sType				= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable		= VK_FALSE,
		.logicOp			= VK_LOGIC_OP_COPY,
		.attachmentCount	= 1,
		.pAttachments		= &(VkPipelineColorBlendAttachmentState)
		{
			.blendEnable			= VK_TRUE,
			.srcColorBlendFactor	= VK_BLEND_FACTOR_SRC_ALPHA,
			.dstColorBlendFactor	= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			.colorBlendOp			= VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor	= VK_BLEND_FACTOR_ONE,
			.dstAlphaBlendFactor	= VK_BLEND_FACTOR_ZERO,
			.alphaBlendOp			= VK_BLEND_OP_ADD,
			.colorWriteMask			= VK_COLOR_COMPONENT_R_BIT
									  | VK_COLOR_COMPONENT_G_BIT
									  | VK_COLOR_COMPONENT_B_BIT
									  | VK_COLOR_COMPONENT_A_BIT,
		},
	};

	const VkPipelineDynamicStateCreateInfo dys_info = 
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = (VkDynamicState[2])
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
		},
	};

	const VkPipelineLayoutCreateInfo lay_info = 
	{
		.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount			= 0,
		.pushConstantRangeCount	= 0,
	};

	if (vkCreatePipelineLayout(app->device, &lay_info, NULL, &app->pp_layout) != VK_SUCCESS)
		app_panic("failed to create pipeline layout.");
	
	const VkPipelineRenderingCreateInfo rend_info =
	{
		.sType						= VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount		= 1,
		.pColorAttachmentFormats	= &app->swap_format,
	};

	const VkGraphicsPipelineCreateInfo pp_info = 
	{
		.sType					= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount				= 2,
		.pStages				= shader_stages,
		.pVertexInputState		= &vis_info,
		.pInputAssemblyState	= &ias_info,
		.pViewportState			= &vs_info,
		.pRasterizationState	= &rast_info,
		.pMultisampleState		= &mss_info,
		.pColorBlendState		= &cbs_info,
		.pDynamicState			= &dys_info,
		.layout					= app->pp_layout,
		.renderPass				= NULL,
		.pNext					= &rend_info,
	};

	if (vkCreateGraphicsPipelines(app->device, VK_NULL_HANDLE, 1, &pp_info, NULL, &app->pipeline) != VK_SUCCESS)
		app_panic("failed to create graphics pipeline.");

	vkDestroyShaderModule(app->device, shader_module, NULL);
	app->state = VK_PIPELINE;
}
