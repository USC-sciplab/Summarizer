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
#include <vector>
#include <cmath>

#include <cstdint>
#include <unistd.h>
#include "../util/util.h"
using namespace std;

// ******************************************************
#define NUMBER_OF_PAGES_IN_A_COMMAND 32
#define REQUEST_SIZE (4096*NUMBER_OF_PAGES_IN_A_COMMAND)
#define LBA_BASE 0x8000

#define DATABASE_AT_HOST 0
#define DATABASE_AT_ISSD 1
#define THRESHOLD 0.6
// *****************************************************
//void WriteResults();
struct record
{
	int rid;
	int len;
	vector<int> tokenArray;
//	int indexPrefixLength, probePrefixLength;
};
//vector<record> joinRecord;
//int tokenNum = -1;

void ssjoin_baseline(int fd, void *buffer, unsigned int pageCount, record queryRecord);
void ssjoin_ssd_all(int fd, void *buffer, unsigned int pageCount);
void ssjoin_ssd_cmp(int fd, void *buffer, unsigned int pageCount);
void ssjoin_ssd_mix(int fd, void *buffer, unsigned int pageCount, record queryRecord);
