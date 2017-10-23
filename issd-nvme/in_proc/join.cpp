#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <cstdint>
#include <map>
#include <vector>
#include <cmath>
#include <syslog.h>
#include <unordered_map>
using namespace std;

#define THRESHOLD 0.5

#if (HV_PREFIX_SIMILARITY)
extern "C" {
	void dm_df_read_prefix_similarity_read (uint8_t *buffer);
	void dm_df_read_prefix_similarity_finalize ();
}
#endif

#if (HV_PREFIX_SIMILARITY)
struct record
{
	int rid;
	int len;
	vector<int> tokenArray;
};
int KM_prefix_similarity_number_of_matched_query_records;
record queryRecord; 
bool prefix_similarity_verify(record queryRecord, record inputRecord){ 
	//Overlap similarity >= threshold 
//	cout << "bpVerify"<< endl; 
	unsigned int minLen = std::min(queryRecord.len, inputRecord.len); 
	int match = 0; 
    for(int i=0;i<minLen;i++){
        for(int j=0;j<minLen;j++){
            if(queryRecord.tokenArray[i]==inputRecord.tokenArray[j]) match++;
        }
	} 
	double similarity = (double)match/(double)minLen; 
//	cout << "similarity value is: " << similarity << endl; 
//	syslog(LOG_INFO, "similarity value is %lf ",similarity);
	if(similarity>=THRESHOLD) return true; 
	return false; 
} 
int Current_prefix_query_number = 0;
bool is_query_record_initialized = 0;
void dm_df_read_prefix_query_initialization ()
{
	//Empty current record
    queryRecord.tokenArray.clear();
	switch (Current_prefix_query_number) {
		case 0:
			//Initialize the 1st record;
            queryRecord.rid = 612258;
            queryRecord.len = 6;
            queryRecord.tokenArray = {517455,551300,578172,579467,579885,579999};
			break;
		case 1:
            queryRecord.rid = 204603;
            queryRecord.len = 9;
            queryRecord.tokenArray = {403078,577748,577947,578215,579062,579988,579998,580021,580023};
			break;
		case 2:
            queryRecord.rid = 655224;
            queryRecord.len = 11;
            queryRecord.tokenArray = {545863,574309,574751,578093,579262,579284,579502,579736,579887,579894,580016};
			break;
		case 3:
            queryRecord.rid = 175785;
            queryRecord.len = 12;
            queryRecord.tokenArray = {434526,564465,570297,573867,575185,576579,577005,577334,578046,579985,580011,580025};
			break;
		case 4:
            queryRecord.rid = 528795;
            queryRecord.len = 13;
            queryRecord.tokenArray = {562634,573086,573209,574144,576360,577517,579521,579802,579979,579994,579996,580024,580025};
			break;
		case 5:
            queryRecord.rid = 325395;
            queryRecord.len = 14;
            queryRecord.tokenArray = {440447,532038,553642,555951,561811,572715,577260,577800,579717,579861,579926,580012,580021,580025};
			break;
		case 6:
            queryRecord.rid = 550357;
            queryRecord.len = 15;
            queryRecord.tokenArray = {544140,552002,571956,578478,578922,579559,579662,579855,579869,579913,579966,579994,580003,580010,580023};
			break;
		case 7:
            queryRecord.rid = 273590;
            queryRecord.len = 17;
            queryRecord.tokenArray = {553743,566882,571700,572250,575744,576060,576805,577069,577939,578784,579156,579432,579946,579977,580022,580023,580024};
			break;
		case 8:
            queryRecord.rid = 392843;
            queryRecord.len = 18;
            queryRecord.tokenArray = {135550,356724,501229,522514,533439,562176,564597,572682,574728,578317,578462,578531,579248,579538,579786,579958,579969,579983};
			break;
		case 9:
            queryRecord.rid = 373774;
            queryRecord.len = 20;
            queryRecord.tokenArray = {438598,452094,540277,558188,575438,576849,577224,579140,579157,579346,579396,579738,579741,579866,579938,579989,580012,580017,580023,580024};
			break;
	}
	Current_prefix_query_number = (Current_prefix_query_number + 1) % 10;
}
void dm_df_read_prefix_similarity_read (uint8_t *buffer){ 
	if(is_query_record_initialized == 0)
	{
		//Initialize 1st record here
        queryRecord.tokenArray.clear();
        queryRecord.rid = 612258;
        queryRecord.len = 6;
        queryRecord.tokenArray = {517455,551300,578172,579467,579885,579999};
		is_query_record_initialized = 1;
	}
	//Increment the number of matches here
    
	//void baseline(int fd, void *buffer, unsigned int pageCount, record queryRecord) 
	int err; 
	int totalLen = 0; 
	int inputToken; 
	record inputRecord; 
	vector<record> joinRecord; 
	bool match = false; 
	bool similar = false; 
	int incrMatch = 0; 
	int incrSimilar = 0; 
	int recordNum = 0; 

	int *recordID = (int *)calloc(1,sizeof(int)); 
	int *recordLen = (int *)calloc(1,sizeof(int)); 
	int *token = (int *)calloc(1,sizeof(int)); 
	joinRecord.clear(); 

	int offset = 0; 
	int shift = 0; 
	int inputPrefixLength = 0; 
	int queryPrefixLength = 0; 
	//cout << "bp1" << endl; 
	int temp_count_remove_variable = 0;
	while(1) 
	{ 
		
		memcpy((char *)recordID, (char *)(((char *)buffer)+offset), sizeof(int));  
		memcpy((char *)recordLen, (char *)(((char *)buffer)+offset+sizeof(int)), sizeof(int)); 
		inputRecord.rid = *recordID; 
		inputRecord.len = *recordLen; 
		inputRecord.tokenArray.clear(); 
		totalLen += *recordLen; 
		for(int ii = 0; ii < *recordLen; ii++){ 
			memcpy((char *)token, (char *)(((char *)buffer)+offset+(2+ii)*sizeof(int)), sizeof(int)); 
			//if (token > tokenNum) tokenNum = token; 
			inputRecord.tokenArray.push_back(*token); 
		}
/*		if(temp_count_remove_variable < 3)
		{
//			syslog(LOG_INFO, "record len id %d %d", *recordLen, *recordID);
			for(auto it = inputRecord.tokenArray.begin(); it != inputRecord.tokenArray.end(); it++)
			{
				syslog(LOG_INFO, "%d ",*it);
			}
			temp_count_remove_variable++;

		}
		if(*recordID==655224){
			syslog(LOG_INFO, "Len for 655224 %d ",*recordLen);
		}
		if(*recordID==528795){
			syslog(LOG_INFO, "Len for 655224 %d ",*recordLen);
		}
		if(*recordID==550357){
			syslog(LOG_INFO, "Len for 655224 %d ",*recordLen);
		}
		if(*recordID==392843){
			syslog(LOG_INFO, "Len for 655224 %d ",*recordLen);
		}
*/		// Prefix filtering
		match = false;
		inputPrefixLength = inputRecord.len - std::ceil(THRESHOLD*(inputRecord.len)) + 1;
		queryPrefixLength = queryRecord.len - std::ceil(THRESHOLD*(queryRecord.len)) + 1;
		for(int jj = 0; jj < inputPrefixLength; jj++){
			for(int kk = 0; kk < queryPrefixLength; kk++){
				//cout << "bp2:RecordID "<< inputRecord.rid << endl;
				if(inputRecord.tokenArray[jj] == queryRecord.tokenArray[kk]){
					match = true;
					break;
				}
			}
		}
		if(match){
			incrMatch++;
//			syslog(LOG_INFO, "Match record id %d %d", inputRecord.rid, queryRecord.rid);
			similar = prefix_similarity_verify(queryRecord, inputRecord);
		}
		if(similar){
			incrSimilar++;

            KM_prefix_similarity_number_of_matched_query_records++;
//			syslog(LOG_INFO, "Similar  %d %d", incrSimilar, KM_prefix_similarity_number_of_matched_query_records);
			//cout <<  "Found match for input Record:" << inputRecord.rid <<" to queryRecord:" << queryRecord.rid << endl;
		}
		similar = false;
		joinRecord.push_back(inputRecord);
		++recordNum;
		shift  = (2+(*recordLen))*(sizeof(int));
		offset = offset + shift;
		if((offset + shift) >= (4096)){
			offset = 0;
			break;
		}
	}
	/* output statistics */
	free(recordID);
	free(recordLen);
	free(token);
}
void dm_df_read_prefix_similarity_finalize()
{
	dm_df_read_prefix_query_initialization();
	//Make sure the all the data structures  are freed
	KM_prefix_similarity_number_of_matched_query_records = 0;
}
#endif
