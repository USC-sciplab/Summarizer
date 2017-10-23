#include "nvme_ssjoin.h"

FILE *pFile;                                                                             
size_t result;  
exec_time_t acc_et_io, acc_et_proc;
unsigned int proc_delay;
unsigned int n_ssd_proc;

int main(int argc, char **argv)
{
	static const char *perrstr;
	int err, fd;
	struct nvme_user_io io;

	if (argc != 4) {
		fprintf(stderr, "Usage: %s <device> <pageCount> <proc_delay>\n", argv[0]);
		return 1;
	}
	perrstr = argv[1];
	fd = open(argv[1], O_RDWR);
	if (fd < 0) {
		printf("Error: cannot open %s\n", argv[1]);
		exit(1);
	}
	unsigned int pageCount = atoi(argv[2]);
	proc_delay = 0;
	proc_delay = atoi(argv[3]);
	n_ssd_proc = 0;

	long lSize;
	void * buffer;
	
	//Read from the file and write to the nvmeSSD
	//Read from the nvmeSSD and the process
	/*We pre-process the linetable using another program and store it in a program relavant to us. The format is (Record1, Record2, Record3,....until 4096 bytes have filled up, records don't cross 4096 bytes boundaries),(Recordi, Recordi+1, ... until 4096 bytes), ....
	The pre-processed database we write it to the SSD. Each 4096 byte block goes to different logical block.
	*/

	if (posix_memalign(&buffer, 4096, REQUEST_SIZE)) {
		fprintf(stderr, "can not allocate io payload\n");
		return 0;
	}

	/*Read the values written and process each block by block*/
	/*Things to do: 1) Experiment with various read lengths */
    FILE *QueryFilePtr;
    QueryFilePtr = fopen( "dblp.query5.txt" , "r");
    if (QueryFilePtr==NULL) {fputs ("Query File error",stderr); exit (1);}
    unsigned int NumberOfQueries;
    int token, recordLen, recordID;
    fscanf(QueryFilePtr, "%u", &NumberOfQueries);
    record queryRecord;

	exec_time_t et;
	char et_name[64];
	strcpy(et_name, "overall execution time");
	init_exec_time(&et, et_name);

	strcpy(et_name, "overall io");
	init_exec_time(&acc_et_io, et_name);
	strcpy(et_name, "overall proc");
	init_exec_time(&acc_et_proc, et_name);
    
    for(unsigned int IterationForQuery = 0; IterationForQuery < NumberOfQueries; IterationForQuery++)
    {
	    unsigned int temp;
        fscanf(QueryFilePtr, "%d %d", &recordID, &recordLen);
        queryRecord.tokenArray.clear();
        queryRecord.rid = recordID;
        queryRecord.len = recordLen;
        for(int k=0; k<recordLen; k++){
            fscanf(QueryFilePtr, "%d", &token);
            queryRecord.tokenArray.push_back(token);
        }

		set_start_time(&et);
	#if (SSD_PROC_MODE==0)	// baseline
	    ssjoin_baseline(fd, buffer, pageCount, queryRecord);
	#endif
	#if (SSD_PROC_MODE==2)	// baseline
	    ssjoin_ssd_cmp(fd, buffer, pageCount);
	#endif
	#if (SSD_PROC_MODE==1)	// baseline
	    ssjoin_ssd_all(fd, buffer, pageCount);
	#endif	
	#if (SSD_PROC_MODE==3)	// baseline
	    ssjoin_ssd_mix(fd, buffer, pageCount, queryRecord);
	#endif
	    cout << pageCount << endl;
        cout << IterationForQuery << endl;
    }

	print_exec_time(&et);
	print_exec_time(&acc_et_io);
	print_exec_time(&acc_et_proc);

	fclose(QueryFilePtr);
	free(buffer);

return 0;

//perror:
//	perror(perrstr);

return 1;
}
