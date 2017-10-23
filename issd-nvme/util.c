/*
 * =====================================================================================
 *
 *       Filename:  util.c
 *    Description:  
 *
 *         Author:  Gunjae Koo (gunjae.koo@gmail.com), 
 *
 * =====================================================================================
 */

#include "util.h"

/// Measure execution time
void init_exec_time( exec_time_t *t, char *name )
{
	clock_gettime(CLOCK_MONOTONIC, &(t->start));
	clock_gettime(CLOCK_MONOTONIC, &(t->bw_start));
	t->time_elapsed = 0.0;
	t->iter = 0;
	strcpy(t->name, name);
}

void init_time( exec_time_t *t )
{
	clock_gettime(CLOCK_MONOTONIC, &(t->start));
	clock_gettime(CLOCK_MONOTONIC, &(t->bw_start));
	t->time_elapsed = 0.0;
	t->iter = 0;
}

void set_start_time( exec_time_t *t )
{
	clock_gettime(CLOCK_MONOTONIC, &(t->start));
}

void add_exec_time( exec_time_t *t )
{
	clock_gettime(CLOCK_MONOTONIC, &(t->end));
	
	double time;
	time = ((double)t->end.tv_sec - (double)t->start.tv_sec);
	time += ((double)t->end.tv_nsec - (double)t->start.tv_nsec) / GIGA;

	t->iter++;
	t->time_elapsed += time;
}

void print_exec_time( exec_time_t *t )
{
	GK_PRINT("TIME %s: total time = %lf (s), iter = %u, time/iter = %lf  (us)\n",
			t->name, t->time_elapsed, t->iter, t->time_elapsed / t->iter*1e6);
	//fprintf(stdout, "TIME %s: total time = %lf, iter = %u, time/iter = %lf \n",
	//		t->name, t->time_elapsed, t->iter, t->time_elapsed / t->iter);
}

void print_exec_bw( exec_time_t *t )
{
	struct timespec end;
	clock_gettime(CLOCK_MONOTONIC, &end);
	
	double time;
	time = ((double)end.tv_sec - (double)t->bw_start.tv_sec);
	time += ((double)end.tv_nsec - (double)t->bw_start.tv_nsec) / GIGA;
	
	GK_PRINT("BW %s: total time = %lf (s), iter = %u, iter/time = %lf (kT/s)\n",
			t->name, time, t->iter, t->iter / time / 1e3);
}
