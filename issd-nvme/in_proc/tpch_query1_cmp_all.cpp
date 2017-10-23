#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <cstdint>
#include <map>
#include <unordered_map>
#include <syslog.h>
using namespace std;

#if (KM_TPCH_QUERY1_CMP_ALL)
extern "C" {
void dm_df_read_tpch_query1_cmp_all_read(uint8_t *buf);
}

extern char KM_tpch_query1_date[20];
uint8_t * KM_tpch_query1_cmp_all_buffer;
unsigned int KM_tpch_query1_cmp_all_buffer_pageNum, KM_tpch_query1_cmp_all_buffer_page_position;
unsigned int KM_tpch_query1_cmp_all_numRecords;

void convert_tpch_query1_cmp_all(char *date, int &year, int &month, int &day);
bool compareDates_tpch_query1_cmp_all(char *str1, char *str2, int interval);

void dm_df_read_tpch_query1_cmp_all_read(uint8_t *buf)
{
	char *l_shipdate = (char *)calloc(9,1);

	int offset = 0;
	while(1)
	{
		memcpy((char *)l_shipdate, (char *)(((char *)buf)+offset+50), 8);
		if((compareDates_tpch_query1_cmp_all(KM_tpch_query1_date, l_shipdate, 0) == 1) or (strcmp(l_shipdate, KM_tpch_query1_date) == 0))
		{
			unsigned int position = KM_tpch_query1_cmp_all_buffer_page_position;
			if(KM_tpch_query1_cmp_all_buffer_pageNum == 0) position += 4;
			if(position + 34 >= 4096)
			{
				KM_tpch_query1_cmp_all_buffer_pageNum += 1;
				KM_tpch_query1_cmp_all_buffer_page_position = 0;
				position = 0;
			}
			memcpy((char *)(KM_tpch_query1_cmp_all_buffer+KM_tpch_query1_cmp_all_buffer_pageNum*4096 + position), (char *)(buf+offset+48), 2);
//			syslog(LOG_INFO, "%c%c",*(KM_tpch_query1_cmp_all_buffer+KM_tpch_query1_cmp_all_buffer_pageNum*4096 + position), *(KM_tpch_query1_cmp_all_buffer+KM_tpch_query1_cmp_all_buffer_pageNum*4096 + position+1) );
			memcpy((char *)(KM_tpch_query1_cmp_all_buffer+KM_tpch_query1_cmp_all_buffer_pageNum*4096 + position+2), (char *)(buf+offset+32), 8);
			memcpy((char *)(KM_tpch_query1_cmp_all_buffer+KM_tpch_query1_cmp_all_buffer_pageNum*4096 + position+10), (char *)(buf+offset+16), 8);
			memcpy((char *)(KM_tpch_query1_cmp_all_buffer+KM_tpch_query1_cmp_all_buffer_pageNum*4096 + position+18), (char *)(buf+offset+24), 8);
			memcpy((char *)(KM_tpch_query1_cmp_all_buffer+KM_tpch_query1_cmp_all_buffer_pageNum*4096 + position+26), (char *)(buf+offset+40), 8);

			KM_tpch_query1_cmp_all_buffer_page_position += 34;
			KM_tpch_query1_cmp_all_numRecords += 1;
		}

		offset += 153;
		if((offset + 153) >= (4096) )
		{
			offset = 0;
			break;
		}
	}

	free(l_shipdate);
}

void convert_tpch_query1_cmp_all(char *date, int &year, int &month, int &day)
{
	char *Year = (char *)calloc(5,1);
	memcpy((void *)Year, (void *)date, 4);
	year = atoi(Year);
	char *Month = (char *)calloc(3,1);
	memcpy((void *)Month, (void *)(date+4), 2);
	month = atoi(Month);
	char *Day = (char *)calloc(3,1);
	memcpy((void *)Day, (void *)(date+6), 2);
	day = atoi(Day);
}

bool compareDates_tpch_query1_cmp_all(char *str1, char *str2, int interval)
{
	int year1, year2, month1, month2, day1, day2;
	convert_tpch_query1_cmp_all(str1, year1, month1, day1);
	convert_tpch_query1_cmp_all(str2, year2, month2, day2);
	if(interval == 0)
	{
		if(year2 < year1) return 1;
		else if(year2 > year1) return 0;
		else {
			if(month2 < month1) return 1;
			else if(month2 > month1) return 0;
			else {
				if(day2 < day1) return 1;
				else if(day2 > day1) return 0;
				else {
					return 0;
				}
			}
		}
	}
	else {
		if(year2 < (year1+interval)) return 1;
		else if(year2 > (year1+interval)) return 0;
		else {
			if(month2 < month1) return 1;
			else if(month2 > month1) return 0;
			else {
				if(day2 < day1) return 1;
				else if(day2 > day1) return 0;
				else {
					return 0;
				}
			}
		}
	}
}

#endif // (KM_TPCH_QUERY1 || KM_TPCH_QUERY1_CMP_ALL)
