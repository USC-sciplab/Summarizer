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
void init_exec_time( exec_time_t *t, char *name)
{
	clock_gettime(CLOCK_MONOTONIC, &(t->start));
	t->time_elapsed = 0.0;
	t->iter = 0;
	strcpy(t->name, name);
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

void accumulate_exec_time( exec_time_t *a, exec_time_t *b )
{
	a->iter += b->iter;
	a->time_elapsed += b->time_elapsed;
}

void print_exec_time( exec_time_t *t )
{
	//fprintf(stdout, "TIME %s: total time = %lf (s), iter = %u, time/iter = %lf (s)\n",
	//		t->name, t->time_elapsed, t->iter, t->time_elapsed / t->iter);
	fprintf(stdout, "TIME %s: total time = %lf (s), iter = %u, time/iter = %lf (us)\n",
			t->name, t->time_elapsed, t->iter, t->time_elapsed / t->iter * 1e6);
}
