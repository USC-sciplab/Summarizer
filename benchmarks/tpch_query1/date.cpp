#include "nvme_tpch_query1.h"

void ReduceDate(char *buffer, unsigned int interval)
{
	if(interval == 60) memcpy(buffer, "19981002", 9);
	else if(interval == 61) memcpy(buffer, "19981001", 9);
	else if((interval >= 62) && (interval <= 91)) {sprintf(buffer, "199809%02u", 91+1-interval);printf("199809%02u\n", 91+1-interval);}
	else if((interval >= 92 && interval <= 120)) {sprintf(buffer, "199808%02u", 31-(interval-92));printf(buffer, "199808%02u\n", 31-(interval-92));}
	return;
}
