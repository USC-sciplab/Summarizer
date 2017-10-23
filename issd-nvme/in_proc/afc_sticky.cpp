#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <cstdint>
#include <map>
#include <unordered_map>
#include <syslog.h>
using namespace std;

#if (TE_AFC_STICKY)
extern "C" {
void dm_df_afc_sticky_read(uint8_t *buf);
}

// Variables
std::unordered_map<int, int> item_frequency;
float TE_sampling_rate;
int TE_minCount;
int TE_results[100];

// Handle a page of elements given sampling rate
void dm_df_afc_sticky_read(uint8_t *buf) 
{
//	syslog(LOG_INFO, "In page %d", item_frequency.size());
	int * item_id_ptr = (int *) calloc(1, sizeof(int));
	int offset = 0;
	while (1) {
		memcpy((char *)item_id_ptr, (char *)(((char*)buf) + offset + 0), 4);
		if (item_frequency.find(*item_id_ptr) != item_frequency.end() && 
				item_frequency[*item_id_ptr] > 0) {
			++item_frequency[*item_id_ptr];
		}
		else if (((double)rand())/RAND_MAX <= (1.0/TE_sampling_rate)) {
			item_frequency[*item_id_ptr] = 1;
		}

		offset += 4;
		if (offset + 4 >= 4096) {
			offset = 0;
			break;
		}
	}
	free(item_id_ptr);
}

// Helper functions
extern "C" {
void TE_afc_update_entries();
void TE_afc_get_results();
void TE_afc_free_hash_table();
}

// Update hash table entries 
void TE_afc_update_entries()
{
//	syslog(LOG_INFO, "Entering in update entries");
	for (auto p = item_frequency.begin(); p != item_frequency.end(); ++p) {
		while (p->second > 0) {
			if (rand() % 2) --(p->second);
			else break;
		}
	}
	return;
}
// Get the item ids which have occurrence above 'minCount'
void TE_afc_get_results()
{
	int idx = 0;
	for (auto p = item_frequency.begin(); p != item_frequency.end(); ++p) {
		if (p->second > TE_minCount) {
			TE_results[idx++] = p->first;
		}
	}
//	syslog(LOG_INFO, " Number of frequent items = %d", idx);
	// Fill the results with -1
	for (; idx < 100; ++idx) TE_results[idx] = -1;
	return;
}
// Free hash table
void TE_afc_free_hash_table() 
{
	//for (auto p = item_frequency.begin(); p != item_frequency.end(); ++p) {
	//	free(p->second);
	//}
	item_frequency.clear();
	//free(TE_results);
	return;
}
#endif // TE_AFC_STICKY
