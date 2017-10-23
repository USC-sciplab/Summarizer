#ifndef __TPCH_Q14_H__
#define __TPCH_Q14_H__

#include <linux/nvme.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <map>
#include <unordered_map>

#include <cstdint>
#include <unistd.h>
#include "../util/util.h"

using namespace std;

// *********************************************
//	Configurations
#define NUMBER_OF_PAGES_IN_A_COMMAND 32
#define REQUEST_SIZE (4096*NUMBER_OF_PAGES_IN_A_COMMAND)
#define LBA_BASE 0x201
// *********************************************

void convert(char *date, int &year, int &month, int &day);
bool compareDates(char *str1, char *str2, int interval);

void tpch_query14_baseline(char *date, int fd, void *buffer, unsigned int pageCount1, unsigned int pageCount2, double & promo_revenue, double & revenue);
void tpch_query14_ssd_all(char *date, int fd, void *buffer, unsigned int pageCount1, unsigned int pageCount2, double & promo_revenue, double & revenue);
void tpch_query14_ssd_cmp(char *date, int fd, void *buffer, unsigned int pageCount1, unsigned int pageCount2, double & promo_revenue, double & revenue);
void tpch_query14_ssd_mix(char *date, int fd, void *buffer, unsigned int pageCount1, unsigned int pageCount2, double & promo_revenue, double & revenue);
#endif
