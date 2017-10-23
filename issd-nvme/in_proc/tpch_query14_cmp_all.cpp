#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <cstdint>
#include <map>
using namespace std;

#if (KM_TPCH_QUERY14_CMP_ALL)
extern "C" {
void dm_df_read_tpch_query14_cmp_all_read1(uint8_t *buf);
}

uint8_t * KM_tpch_query14_cmp_all_buffer;
unsigned int KM_tpch_query14_cmp_all_buffer_pageNum, KM_tpch_query14_cmp_all_buffer_page_position;
unsigned int KM_tpch_query14_cmp_all_numRecords;

void dm_df_read_tpch_query14_cmp_all_read1(uint8_t *buf)
{
	int offset2=0;
	while(1)
	{
		unsigned int position = KM_tpch_query14_cmp_all_buffer_page_position;
		if(KM_tpch_query14_cmp_all_buffer_pageNum == 0) position += 4;
		if(position + 13 >= 4096)
		{
			KM_tpch_query14_cmp_all_buffer_pageNum += 1; 
			KM_tpch_query14_cmp_all_buffer_page_position = 0;
			position = 0;
		}
		memcpy((char *)(KM_tpch_query14_cmp_all_buffer+KM_tpch_query14_cmp_all_buffer_pageNum*4096 + position), (char *)(buf+offset2+0), 8);
		memcpy((char *)(KM_tpch_query14_cmp_all_buffer+KM_tpch_query14_cmp_all_buffer_pageNum*4096 + position + 8), (char *)(((char *)buf)+offset2+99), 5);
		KM_tpch_query14_cmp_all_buffer_page_position += 13;
		KM_tpch_query14_cmp_all_numRecords += 1;

		offset2 += 168;
		if((offset2 + 168) >= 4096)
		{
			offset2 = 0;
			break;
		}
	}
}

extern char KM_tpch_query14_date[20];
extern bool compareDates_tpch_query14(char *str1, char *str2, int interval);
extern void convert_tpch_query14(char *date, int &year, int &month, int &day);

extern "C" {
void dm_df_read_tpch_query14_cmp_all_read2(uint8_t *buf);
}

void dm_df_read_tpch_query14_cmp_all_read2(uint8_t *buf)
{
	char *l_shipdate = (char *)calloc(9,1);

	int offset = 0;
	while(1)
	{
		memcpy((char *)l_shipdate, (char *)(((char *)buf)+offset+50), 8);

		if(((compareDates_tpch_query14(l_shipdate, KM_tpch_query14_date, 0) == 1) or (strcmp(l_shipdate, KM_tpch_query14_date) == 0)) and ((compareDates_tpch_query14(KM_tpch_query14_date, l_shipdate, 1) == 1)))
		{
			unsigned int position = KM_tpch_query14_cmp_all_buffer_page_position;
			if(KM_tpch_query14_cmp_all_buffer_pageNum == 0) position += 4;
			if(position + 20 >= 4096)
			{
				KM_tpch_query14_cmp_all_buffer_pageNum += 1; 
				KM_tpch_query14_cmp_all_buffer_page_position = 0;
				position = 0;
			}
			memcpy((char *)(KM_tpch_query14_cmp_all_buffer+KM_tpch_query14_cmp_all_buffer_pageNum*4096 + position), (char *)(buf+offset+4), 4);
			memcpy((char *)(KM_tpch_query14_cmp_all_buffer+KM_tpch_query14_cmp_all_buffer_pageNum*4096 + position + 4), (char *)(((char *)buf)+offset+32), 8);
			memcpy((char *)(KM_tpch_query14_cmp_all_buffer+KM_tpch_query14_cmp_all_buffer_pageNum*4096 + position + 12), (char *)(((char *)buf)+offset+24), 8);
			KM_tpch_query14_cmp_all_buffer_page_position += 20;
			KM_tpch_query14_cmp_all_numRecords += 1;
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
#endif // KM_TPCH_QUERY14
