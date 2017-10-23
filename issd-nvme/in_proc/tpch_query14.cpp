#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <cstdint>
#include <map>
#include <unordered_map>
using namespace std;

#if (KM_TPCH_QUERY14 || KM_TPCH_QUERY14_CMP_ALL)
char KM_tpch_query14_date[20];
#endif // (KM_TPCH_QUERY14 || KM_TPCH_QUERY14_CMP_ALL)

#if (KM_TPCH_QUERY14)
extern "C" {
void dm_df_read_tpch_query14_read1(uint8_t *buf);
}

std::unordered_map<unsigned long long int, char *> KM_tpch_query14_hashtable;
void dm_df_read_tpch_query14_read1(uint8_t *buf)
{
	unsigned long long int *p_partkey = (unsigned long long int *) calloc(1, sizeof(unsigned long long int));
	char *p_type;
	int offset2=0;
	while(1)
	{
		memcpy((char *)p_partkey, (char *)(((char *)buf)+offset2+0), 8);
		p_type = (char *)calloc(1,6); 
		//				std::string p_type((((char *)buffer)+4096*j2+offset2+98) ,(((char *)buffer)+4096*j2+offset2+103));
		memcpy(p_type, (char *)(((char *)buf)+offset2+99), 5);
		KM_tpch_query14_hashtable.emplace(*p_partkey, p_type);

		offset2 += 168;
		if((offset2 + 168) >= 4096)
		{
			offset2 = 0;
			break;
		}
	}
	free(p_partkey);
}

extern "C" {
void dm_df_read_tpch_query14_read2(uint8_t *buf);
}

bool compareDates_tpch_query14(char *str1, char *str2, int interval);
void convert_tpch_query14(char *date, int &year, int &month, int &day);

double KM_tpch_query14_revenue;
double KM_tpch_query14_promo_revenue;

void dm_df_read_tpch_query14_read2(uint8_t *buf)
{
	char *l_shipdate = (char *)calloc(9,1);
	double *l_discount = (double *)calloc(1,sizeof(double));
	double *l_quantitiy = (double *)calloc(1,sizeof(double));
	double *l_extendedprice = (double *)calloc(1,sizeof(double));
	unsigned int *l_partkey = (unsigned int *) calloc(1, sizeof(unsigned int));

	int offset = 0;
	while(1)
	{
		memcpy((char *)l_shipdate, (char *)(((char *)buf)+offset+50), 8);

		if(((compareDates_tpch_query14(l_shipdate, KM_tpch_query14_date, 0) == 1) or (strcmp(l_shipdate, KM_tpch_query14_date) == 0)) and ((compareDates_tpch_query14(KM_tpch_query14_date, l_shipdate, 1) == 1)))
		{
			memcpy((char *)l_partkey, (char *)(((char *)buf)+offset+4), 4);
			memcpy((char *)l_discount, (char *)(((char *)buf)+offset+32), 8);
			memcpy((char *)l_extendedprice, (char *)(((char *)buf)+offset+24), 8);
			std::unordered_map<unsigned long long int, char *>::iterator it;
			it = KM_tpch_query14_hashtable.find(*l_partkey);
			if(it != KM_tpch_query14_hashtable.end())
			{
				if(strlen(it->second) >= 5)
				{
					if(strncmp(it->second, "PROMO",5) == 0)
					{
						KM_tpch_query14_promo_revenue += *l_extendedprice * (1 - *l_discount);
					}
				}
				KM_tpch_query14_revenue += *l_extendedprice * (1 - *l_discount);
			}
		}

		offset += 153;
		if((offset + 153) >= (4096) )
		{
			offset = 0;
			break;
		}
	}
	free(l_shipdate);
	free(l_discount);
	free(l_extendedprice);
	free(l_quantitiy);
	free(l_partkey);
}

extern "C" {
void KM_tpch_query14_free_hash_table();
}

void KM_tpch_query14_free_hash_table()
{
	for ( auto local_it = KM_tpch_query14_hashtable.begin(); local_it != KM_tpch_query14_hashtable.end(); ++local_it )
		free(local_it->second);
	KM_tpch_query14_hashtable.clear();
	return;
}
#endif //KM_TPCH_QUERY14

#if (KM_TPCH_QUERY14 || KM_TPCH_QUERY14_CMP_ALL)
void convert_tpch_query14(char *date, int &year, int &month, int &day)
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

bool compareDates_tpch_query14(char *str1, char *str2, int interval)
{
	int year1, year2, month1, month2, day1, day2;
	convert_tpch_query14(str1, year1, month1, day1);
	convert_tpch_query14(str2, year2, month2, day2);
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
		if(year2 < (year1)) return 1;
		else if(year2 > (year1)) return 0;
		else {
			if(month2 < (month1+interval)) return 1;
			else if(month2 > (month1+interval)) return 0;
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
#endif // (KM_TPCH_QUERY14 || KM_TPCH_QUERY14_CMP_ALL)
