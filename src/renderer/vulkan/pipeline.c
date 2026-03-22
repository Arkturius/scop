/**
 * pipeline.c
 */

#include <assert.h>

#include <IG_engine.h>
#include <IG_vkcore.h>
#include <IG_renderer.h>
#include <vulkan/vulkan_core.h>

#define	APP_SHADER_PATH	"src/shaders/shader.spv"

void
IG_vk_pipeline(void)
{
	VkShaderModule	shader_module;

	shader_module = IG_vk_shader_module();

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

	VkVertexInputBindingDescription	vib_desc = IG_vk_vertex_get_binding();

	const VkPipelineVertexInputStateCreateInfo vis_info = 
	{
		.sType		= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.vertexAttributeDescriptionCount = 3,
		.pVertexBindingDescriptions = &vib_desc,
		.pVertexAttributeDescriptions = IG_vk_vertex_get_attributes(),
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
			.width		= IG.renderer->swap_extent.width,
			.height		= IG.renderer->swap_extent.height,
			.minDepth	= 0.0f,
			.maxDepth	= 1.0f,
		},
		.pScissors		= &(VkRect2D)
		{
			.offset		= (VkOffset2D){0, 0},
			.extent 	= IG.renderer->swap_extent,
		},
	};

	const VkPipelineRasterizationStateCreateInfo rast_info = 
	{	
		.sType					= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.polygonMode			= VK_POLYGON_MODE_FILL,
		.cullMode				= VK_CULL_MODE_BACK_BIT,
		.frontFace				= VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable		= VK_FALSE,
		.depthBiasSlopeFactor	= 1.0f,
		.lineWidth				= 1.0f,
	};

	const VkPipelineDepthStencilStateCreateInfo depth_info =
	{
		.sType				= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable	= VK_TRUE,
		.depthWriteEnable	= VK_TRUE,
		.depthCompareOp		= VK_COMPARE_OP_LESS,
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
		.setLayoutCount			= 1,
		.pSetLayouts			= &IG.renderer->ds_layout,
		.pushConstantRangeCount	= 0,
	};

	if (vkCreatePipelineLayout(IG.vulkan->device, &lay_info, NULL, &IG.renderer->pp_layout) != VK_SUCCESS)
		IG_panic("failed to create pipeline layout.");
	
	const VkPipelineRenderingCreateInfo rend_info =
	{
		.sType						= VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount		= 1,
		.pColorAttachmentFormats	= &IG.renderer->swap_format,
		.depthAttachmentFormat		= IG_vk_depth_format(),
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
		.pDepthStencilState		= &depth_info,
		.pMultisampleState		= &mss_info,
		.pColorBlendState		= &cbs_info,
		.pDynamicState			= &dys_info,
		.layout					= IG.renderer->pp_layout,
		.renderPass				= NULL,
		.pNext					= &rend_info,
	};

	if (vkCreateGraphicsPipelines(IG.vulkan->device, VK_NULL_HANDLE, 1, &pp_info, NULL, &IG.renderer->pipeline) != VK_SUCCESS)
		IG_panic("failed to create graphics pipeline.");

	vkDestroyShaderModule(IG.vulkan->device, shader_module, NULL);
	IG.state = IG_PIPELINE;
}
