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

#include <sys/ipc.h>
#include <sys/msg.h>
#include <syslog.h>

// gunjae's functions for monitoring
#if (VERBOSE==1)
	#define GK_PRINT(args...) fprintf(stdout, args)
	#define GK_REQ_PRINT(args...) syslog(LOG_INFO, args)
#else
	#define GK_PRINT(args...) syslog(LOG_INFO, args)
	//#define GK_REQ_PRINT(args...) syslog(LOG_INFO, args)
	#define GK_REQ_PRINT(args...)
#endif  // VERBOSE

#define GIGA 1000000000.0

/// Measure execution time
typedef struct exec_time_t {
	struct timespec	start;
	struct timespec end;
	struct timespec bw_start;
	double			time_elapsed;	// sec
	unsigned int	iter;
	char			name[64];
} exec_time_t;

void init_exec_time( exec_time_t *t, char *name );
void init_time( exec_time_t *t );
void set_start_time( exec_time_t *t );
void add_exec_time( exec_time_t *t );
void print_exec_time( exec_time_t *t );
void print_exec_bw( exec_time_t *t );
///

#endif	// _UTIL_H_
