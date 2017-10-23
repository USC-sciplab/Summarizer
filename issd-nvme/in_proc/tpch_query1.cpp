#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <cstdint>
#include <map>
#include <unordered_map>
using namespace std;

#if (KM_TPCH_QUERY1 || KM_TPCH_QUERY1_CMP_ALL)
char KM_tpch_query1_date[20];
#endif // (KM_TPCH_QUERY1 || KM_TPCH_QUERY1_CMP_ALL)

#if (KM_TPCH_QUERY1)
extern "C" {
void dm_df_read_tpch_query1_read(uint8_t *buf);
}

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

std::map <std::string, class storage> KM_tpch_query1_mymap;
void convert_tpch_query1(char *date, int &year, int &month, int &day);
bool compareDates_tpch_query1(char *str1, char *str2, int interval);

void dm_df_read_tpch_query1_read(uint8_t *buf)
{
	char *l_shipdate = (char *)calloc(9,1);
	double *l_discount = (double *)calloc(1,sizeof(double));
	double *l_quantitiy = (double *)calloc(1,sizeof(double));
	double *l_extendedprice = (double *)calloc(1,sizeof(double));
	double *l_tax = (double *) calloc(1,sizeof(double));
	std::string l_returnflagAndLinestatus;
	l_returnflagAndLinestatus.resize(2);

	std::map<char *, class storage>::iterator it;

	int offset = 0;
	while(1)
	{
		memcpy((char *)l_shipdate, (char *)(((char *)buf)+offset+50), 8);
		if((compareDates_tpch_query1(KM_tpch_query1_date, l_shipdate, 0) == 1) or (strcmp(l_shipdate, KM_tpch_query1_date) == 0))
		{	
			memcpy((char *)l_discount, (char *)(((char *)buf)+offset+32), 8);
			memcpy((char *)l_quantitiy, (char *)(((char *)buf)+offset+16), 8);
			memcpy((char *)l_extendedprice, (char *)(((char *)buf)+offset+24), 8);
			memcpy((char *)l_tax, (char *)(((char *)buf)+offset+40), 8);
			memcpy((char *)l_returnflagAndLinestatus.data(), (char *)(((char *)buf)+offset+48), 2);

			if(KM_tpch_query1_mymap.find(l_returnflagAndLinestatus) == KM_tpch_query1_mymap.end())
			{
				//	cout << l_returnflagAndLinestatus << " " << l_shipdate << " " << *l_extendedprice << " " << *l_quantitiy << endl;
				KM_tpch_query1_mymap[l_returnflagAndLinestatus];
				KM_tpch_query1_mymap[l_returnflagAndLinestatus].l_returnflag = l_returnflagAndLinestatus[0];
				KM_tpch_query1_mymap[l_returnflagAndLinestatus].l_linestatus = l_returnflagAndLinestatus[1];
				KM_tpch_query1_mymap[l_returnflagAndLinestatus].sum_qty = *l_quantitiy;
				KM_tpch_query1_mymap[l_returnflagAndLinestatus].sum_base_price = *l_extendedprice;
				KM_tpch_query1_mymap[l_returnflagAndLinestatus].sum_disc_price =  *l_extendedprice * (1 - *l_discount);
				KM_tpch_query1_mymap[l_returnflagAndLinestatus].sum_charge = *l_extendedprice * (1 - *l_discount) *(1 + *l_tax);
				KM_tpch_query1_mymap[l_returnflagAndLinestatus].avg_qty = *l_quantitiy;
				KM_tpch_query1_mymap[l_returnflagAndLinestatus].avg_price = *l_extendedprice;
				KM_tpch_query1_mymap[l_returnflagAndLinestatus].avg_disc = *l_discount;
				KM_tpch_query1_mymap[l_returnflagAndLinestatus].count_order = 1;
			}
			else {
				KM_tpch_query1_mymap[l_returnflagAndLinestatus];
				KM_tpch_query1_mymap[l_returnflagAndLinestatus].sum_qty += *l_quantitiy;
				KM_tpch_query1_mymap[l_returnflagAndLinestatus].sum_base_price += *l_extendedprice;
				KM_tpch_query1_mymap[l_returnflagAndLinestatus].sum_disc_price +=  (*l_extendedprice * (1 - *l_discount));
				KM_tpch_query1_mymap[l_returnflagAndLinestatus].sum_charge += (*l_extendedprice * (1 - *l_discount) *(1 + *l_tax));
				KM_tpch_query1_mymap[l_returnflagAndLinestatus].avg_qty += (*l_quantitiy);
				KM_tpch_query1_mymap[l_returnflagAndLinestatus].avg_price += (*l_extendedprice);
				KM_tpch_query1_mymap[l_returnflagAndLinestatus].avg_disc += *l_discount;
				KM_tpch_query1_mymap[l_returnflagAndLinestatus].count_order += 1;
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
	free(l_tax);
}

extern "C" {
	void KM_tpch_query1_free_map();
}

void KM_tpch_query1_free_map()
{
	KM_tpch_query1_mymap.clear();
	return;
}

extern "C" {
	void KM_tpch_query1_prepare_transfer(uint8_t * Result_transfer_buffer, unsigned int *buffer_page_number);
}

void KM_tpch_query1_prepare_transfer(uint8_t * Result_transfer_buffer, unsigned int *buffer_page_number)
{
	unsigned int temp_size = KM_tpch_query1_mymap.size();
	//Allocate Result_transfer_buffer in an aligned fashion
	memcpy(Result_transfer_buffer, &temp_size, sizeof(unsigned int));
	unsigned int buffer_page_position = 4;
	unsigned int buffer_pageNum = 0; 
	for (auto it=KM_tpch_query1_mymap.begin(); it != KM_tpch_query1_mymap.end(); it++)
	{		
		if(buffer_page_position + (2+66) >= 4096)
		{
			buffer_pageNum += 1;
			buffer_page_position = 0;
		}
		memcpy((char *)(Result_transfer_buffer + buffer_pageNum*4096 + buffer_page_position), (it->first).data(), 2);
		memcpy((char *)(Result_transfer_buffer + buffer_pageNum*4096 + buffer_page_position+2+0), &((it->second).l_returnflag), 1);
		memcpy((char *)(Result_transfer_buffer + buffer_pageNum*4096 + buffer_page_position+2+1), &((it->second).l_linestatus), 1);
		memcpy((char *)(Result_transfer_buffer + buffer_pageNum*4096 + buffer_page_position+2+2), &((it->second).sum_qty), sizeof(double));
		memcpy((char *)(Result_transfer_buffer + buffer_pageNum*4096 + buffer_page_position+2+10), &((it->second).sum_base_price), sizeof(double));
		memcpy((char *)(Result_transfer_buffer + buffer_pageNum*4096 + buffer_page_position+2+18), &((it->second).sum_disc_price),  sizeof(double));
		memcpy((char *)(Result_transfer_buffer + buffer_pageNum*4096 + buffer_page_position+2+26), &((it->second).sum_charge), sizeof(double));
		memcpy((char *)(Result_transfer_buffer + buffer_pageNum*4096 + buffer_page_position+2+34), &((it->second).avg_qty), sizeof(double));
		memcpy((char *)(Result_transfer_buffer + buffer_pageNum*4096 + buffer_page_position+2+42), &((it->second).avg_price), sizeof(double));
		memcpy((char *)(Result_transfer_buffer + buffer_pageNum*4096 + buffer_page_position+2+50), &((it->second).avg_disc), sizeof(double));
		memcpy((char *)(Result_transfer_buffer + buffer_pageNum*4096 + buffer_page_position+2+58), &((it->second).count_order), sizeof(double));
		buffer_page_position += (2+(66));
	}
	memcpy(buffer_page_number, &buffer_pageNum, sizeof(unsigned int));
	return;
}
void convert_tpch_query1(char *date, int &year, int &month, int &day)
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

bool compareDates_tpch_query1(char *str1, char *str2, int interval)
{
	int year1, year2, month1, month2, day1, day2;
	convert_tpch_query1(str1, year1, month1, day1);
	convert_tpch_query1(str2, year2, month2, day2);
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
