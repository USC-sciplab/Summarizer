#ifndef __TPCH_Q1_H__
#define __TPCH_Q1_H__

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

class storage {
	public:
		char l_returnflag;
		char l_linestatus;
		double sum_qty;
		double sum_base_price;
		double sum_disc_price;
		double sum_charge;
		double avg_qty;
		double avg_price;
		double avg_disc;
		double count_order;
};

struct char_cmp {
    bool operator () (const char *a,const char *b) const
    {
//	cout << a << " " << b << " " << (strncmp(a,b,2) == 0) << endl;
        return strncmp(a,b,2) <  0;
    }
};

void convert(char *date, int &year, int &month, int &day);
bool compareDates(char *str1, char *str2, int interval);
void ReduceDate(char *buffer, unsigned int interval);

void tpch_query1_baseline(char *date, int fd, void *buffer, unsigned int pageCount, std::map <std::string , class storage> &mymap);
void tpch_query1_ssd_all(char *date, int fd, void *buffer, unsigned int pageCount, std::map <std::string, class storage> &mymap);
void tpch_query1_ssd_cmp(char *date, int fd, void *buffer, unsigned int pageCount, std::map <std::string, class storage> &mymap);
void tpch_query1_ssd_mix(char *date, int fd, void *buffer, unsigned int pageCount, std::map <std::string, class storage> &mymap);
#endif
