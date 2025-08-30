/**
 * main.c
 */

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <scop.h>

singleton(App);

void
app_shader_cleanup(ShaderFile shader)
{
	if (shader.content)
		munmap(shader.content, shader.size);
}

ShaderFile
app_shader_read(String filepath)
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

int
main(int argc, char **argv)
{
	UNUSED(argc);
	UNUSED(argv);

	App	*app = App_getinstance();

	app->run = app_run;
	app->run();
	return (0);
}
