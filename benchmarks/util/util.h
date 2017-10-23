/*
 * =====================================================================================
 *       Filename:  util.h
 *    Description:  
 *
 *         Author:  Gunjae Koo (gunjae.koo@gmail.com) 
 *
 * =====================================================================================
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#include <time.h>
#include <stdio.h>
#include <string.h>

#define GIGA 1000000000.0

/// Measure execution time
typedef struct exec_time_t {
	struct timespec	start;
	struct timespec end;
	double			time_elapsed;	// sec
	unsigned int	iter;
	char			name[64];
} exec_time_t;

void init_exec_time( exec_time_t *t, char *name);
void set_start_time( exec_time_t *t );
void add_exec_time( exec_time_t *t );
void accumulate_exec_time( exec_time_t *a, exec_time_t *b );
void print_exec_time( exec_time_t *t );
///

#endif	// _UTIL_H_
