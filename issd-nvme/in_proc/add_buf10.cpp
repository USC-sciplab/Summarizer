/*
 * =====================================================================================
 *
 *       Filename:  add_buf10.cpp
 *    Description:  
 *
 *         Author:  Gunjae Koo (gunjae.koo@gmail.com), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <cstdint>
#include <map>
#include <unordered_map>
using namespace std;

extern "C" {
void dm_df_add10(uint8_t *buf);
}

void dm_df_add10(uint8_t *buf)
{
	std::map<int, int> mymap;
	mymap[5];
	mymap[5] = 8;

	// dummy function to add the first 10 bytes
	uint64_t sum = 0;
	uint8_t *p_tmp;
	for (int i=0; i < 10; i++) {
		p_tmp = (buf + i);
	    sum += *p_tmp;
	}
	//printf("sum: %d\n", sum);
	cout << "sum: " << sum << endl;
	memcpy(buf, &sum, sizeof(uint64_t));
}
