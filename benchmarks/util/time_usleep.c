/*
 * =====================================================================================
 *
 *       Filename:  time_usleep.c
 *    Description:  
 *
 *         Author:  Gunjae Koo (gunjae.koo@gmail.com)
 *
 * =====================================================================================
 */

#include "util.h"

#define ITER 1000

int main (int argc, char **argv)
{
	exec_time_t et;
	char et_name[64];
	unsigned int i;
	
	// usleep(1)
	strcpy(et_name, "usleep(1)");
	init_exec_time(&et, et_name);
	for (i = 0; i < ITER; i++) {
		set_start_time(&et);
		usleep(1);
		add_exec_time(&et);
	}
	print_exec_time(&et);

	// usleep(10)
	strcpy(et_name, "usleep(10)");
	init_exec_time(&et, et_name);
	for (i = 0; i < ITER; i++) {
		set_start_time(&et);
		usleep(10);
		add_exec_time(&et);
	}
	print_exec_time(&et);

	// usleep(100)
	strcpy(et_name, "usleep(100)");
	init_exec_time(&et, et_name);
	for (i = 0; i < ITER; i++) {
		set_start_time(&et);
		usleep(100);
		add_exec_time(&et);
	}
	print_exec_time(&et);

	// usleep(1000)
	strcpy(et_name, "usleep(1000)");
	init_exec_time(&et, et_name);
	for (i = 0; i < ITER; i++) {
		set_start_time(&et);
		usleep(1000);
		add_exec_time(&et);
	}
	print_exec_time(&et);

	return 0;
}
