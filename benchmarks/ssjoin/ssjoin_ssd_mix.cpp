#include "nvme_ssjoin.h"
#define DATABASE_AT_HOST 0
#define DATABASE_AT_ISSD 1
//extern FILE *pFile;                                                                             
extern size_t result;

extern exec_time_t acc_et_io, acc_et_proc;
extern unsigned int proc_delay;
extern unsigned int n_ssd_proc;

bool verify(record queryRecord, record inputRecord);
//{
//    //Overlap similarity >= threshold
//    //cout << "bpVerify"<< endl;
//	unsigned int minLen = std::min(queryRecord.len, inputRecord.len);
//    int match = 0;
//    for(int i=0;i<minLen;i++){
//        for(int j=0;j<minLen;j++){
//            if(queryRecord.tokenArray[i]==inputRecord.tokenArray[j]) match++;
//        }
//    }
//    double similarity = (double)match/(double)minLen;
//    //cout << "similarity value is: " << similarity << endl;
//    if(similarity>=THRESHOLD) return true;
//    return false;
//}
void ssjoin_ssd_mix(int fd, void *buffer, unsigned int pageCount, record queryRecord)
{
	int err;
	int totalLen = 0;
	int inputToken;
	record inputRecord;
    vector<record> joinRecord;
	struct nvme_user_io io;
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

	exec_time_t et_io, et_proc;
	char et_name[64];
	strcpy(et_name, "io");
	init_exec_time(&et_io, et_name);
	strcpy(et_name, "proc");
	init_exec_time(&et_proc, et_name);

	uint32_t *l_return_code = (uint32_t*)malloc( 1*sizeof(uint32_t) );

    //cout << "bp1" << endl;
	for(unsigned int i=0; i < pageCount; i += NUMBER_OF_PAGES_IN_A_COMMAND)
	{
		unsigned int pageInThisIteration = std::min((unsigned int)NUMBER_OF_PAGES_IN_A_COMMAND, (pageCount-i));
		// copy the file into the buffer:
		io.opcode = nvme_cmd_read;
		io.flags = 18;
		io.control = 0;
		io.metadata = (unsigned long) 0;
		io.addr = (unsigned long) buffer;
		io.slba = LBA_BASE+i;
		io.nblocks = pageInThisIteration-1;
		io.dsmgmt = 0;
		io.reftag = 0;
		io.apptag = 0;
		io.appmask = 0;

		set_start_time(&et_io);
		err = ioctl(fd, NVME_IOCTL_SUBMIT_IO, &io);
		add_exec_time(&et_io);
		//		if (err < 0)
		//			goto perror;
		if (err)
			fprintf(stderr, "nvme read status:%x\n", err);
		for(unsigned int j = 0; j < pageInThisIteration; j++)
		{
			memcpy( (uint8_t*)l_return_code, (uint8_t*)( (uint8_t*)buffer + 4096*j ), sizeof (uint32_t) );
			if ( *l_return_code==(uint32_t)-1) {
				n_ssd_proc++;
				continue;
			}
			set_start_time(&et_proc);
			if (proc_delay > 0)
				usleep(proc_delay);

			while(1)
			{
				memcpy((char *)recordLen, (char *)(((char *)buffer)+4096*j+offset+sizeof(int)), sizeof(int));
				memcpy((char *)recordID, (char *)(((char *)buffer)+4096*j+offset), sizeof(int)); 
                inputRecord.rid = *recordID;
                inputRecord.len = *recordLen;
                inputRecord.tokenArray.clear();
		        totalLen += *recordLen;
				for(int ii = 0; ii < *recordLen; ii++){
					memcpy((char *)token, (char *)(((char *)buffer)+4096*j+offset+(2+ii)*sizeof(int)), sizeof(int));
			        //if (token > tokenNum) tokenNum = token;
        			inputRecord.tokenArray.push_back(*token);
				}
                // Prefix filtering
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
                    similar = verify(queryRecord, inputRecord);
                }
                if(similar){
                    incrSimilar++;
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
			add_exec_time(&et_proc);
		}
	}
	/* output statistics */
    cout << "# Matches: " << incrMatch << endl;
    cout << "# Verified: " << incrSimilar << endl;
	cerr << "# Records: " << recordNum << endl;	
	cerr << "# Average Record Size: " << double(totalLen) / recordNum << endl;	
	cerr << "# Maximum Record Size: " << joinRecord[recordNum - 1].len << endl;

	print_exec_time(&et_io);
	print_exec_time(&et_proc);
	accumulate_exec_time(&acc_et_io, &et_io);
	accumulate_exec_time(&acc_et_proc, &et_proc);

	free(recordID);
	free(recordLen);
	free(token);
}
