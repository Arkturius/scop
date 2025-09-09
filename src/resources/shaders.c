/**
 * shaders.c
 */

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <IG_engine.h>
#include <IG_vkcore.h>
#include <IG_renderer.h>

# define	IG_SHADER_PATH	"src/shaders/shader.spv"

void
IG_shader_cleanup(ShaderFile shader)
{
	if (shader.content)
		munmap(shader.content, shader.size);
}

ShaderFile
IG_shader_read(String filepath)
{
	int			fd				= open(filepath, O_RDONLY);
	struct stat	st				= {0};
	String		file_content	= NULL;

	if (fd == -1)
	{
		warning("shader file '%s' can't be opened.", filepath);
		goto file_close;
	}
	if (fstat(fd, &st) == -1)
	{
		warning("fstat failed: %s.", strerror(errno));
		goto file_close;
	}
	if (st.st_size == 0)
	{
		warning("empty shader file.");
		goto file_close;
	}
	file_content = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (file_content == MAP_FAILED)
	{
		warning("mmap failed: %s.", strerror(errno));
		goto file_close;
	}

file_close:
	close(fd);
	return (ShaderFile)
	{
		.content = file_content != MAP_FAILED ? file_content : NULL,
		.size = file_content ? st.st_size : 0,
	};
}

VkShaderModule 
IG_vk_shader_module(void)
{
	ShaderFile		shader_file;
	VkShaderModule	shader;

	shader_file = IG_shader_read(IG_SHADER_PATH);
	if (!shader_file.content)
		IG_panic("failed to load shader.");

	assert(((uptr)shader_file.content & (sizeof(uint32_t) - 1)) == 0);

	const VkShaderModuleCreateInfo	create_info = 
	{
		.sType		= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize	= shader_file.size,
		.pCode		= (uint32_t *)shader_file.content,
	};

	VkResult	ret = vkCreateShaderModule(IG.vulkan->device, &create_info, NULL, &shader);
	
	IG_shader_cleanup(shader_file);
	if (ret != VK_SUCCESS)
		IG_panic("failed to create shader module.");

	return (shader);
}
