/**
 * main.c
 */

#include <IG_engine.h>

int
main(int argc, char **argv)
{
	unused(argc);
	unused(argv);

	JOBfile	file = job_open_model("viking_room.obj");
	JOBdata	data = {0};

	job_parse_model(&file, &data);

	arr_foreach(JOBv, v, data.v)
	{
		printf("VERTEX [ %f %f %f ]\n", v->x, v->y, v->z);
	}

	IG_run();
	return (0);
}
