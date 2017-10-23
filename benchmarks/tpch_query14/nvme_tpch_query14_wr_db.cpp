// ******************************
//  Write db in SSD
// ******************************

#include "nvme_tpch_query14.h"
#define VERBOSE 1

int main(int argc, char **argv)
{
	static const char *perrstr;
	int err, fd;
	struct nvme_user_io io;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <device>\n", argv[0]);
		return 1;
	}

	perrstr = argv[1];
	fd = open(argv[1], O_RDWR);
	if (fd < 0)
	{
		printf("Error in commandline\n");
	//	goto perror;
	}

	FILE *pFile1, *pFile2;
	long lSize;
	void *buffer;
	size_t result;
	
	//Read from the file and write to the nvmeSSD
	//Read from the nvmeSSD and the process
	/*We pre-process the linetable using another program and store it in a program relavant to us. The format is (Record1, Record2, Record3,....until 4096 bytes have filled up, records don't cross 4096 bytes boundaries),(Recordi, Recordi+1, ... until 4096 bytes), ....
	The pre-processed database we write it to the SSD. Each 4096 byte block goes to different logical block.
	*/

//	pFile = fopen ( "test.db" , "rb" );
	pFile1 = fopen ( "../tpch_db/lineitem.db" , "rb" );
	if (pFile1==NULL) {fputs ("File error",stderr); exit (1);}
	pFile2 = fopen ( "../tpch_db/part.db" , "rb" );
	if (pFile2==NULL) {fputs ("File error",stderr); exit (1);}

	// obtain file size:
	fseek (pFile1 , 0 , SEEK_END);
	lSize = ftell (pFile1);
	rewind (pFile1);

#if (VERBOSE)
	cout << "Size of the file is = " << lSize << "\n" ; 
#endif

	if (posix_memalign(&buffer, 4096, REQUEST_SIZE)) {
		fprintf(stderr, "can not allocate io payload\n");
		return 0;
	}

	// For measure time (need util.h)
	exec_time_t et;
	char et_name[64];
	strcpy(et_name, "db_wr");
	init_exec_time(&et, et_name);

	unsigned int pageCount1 = 0;
	unsigned int pageCount2 = 0;
	
	// define opcode
	io.opcode = nvme_cmd_write;
	io.flags = 0;
	io.control = 0;
	io.metadata = (unsigned long) 0;
	io.addr = (unsigned long) buffer;
	io.slba = LBA_BASE + pageCount1;
	io.nblocks = 0;
	io.dsmgmt = 0;
	io.reftag = 0;
	io.apptag = 0;
	io.appmask = 0;
	
	while(1)
	{
		result = fread (buffer,1,4096,pFile1);
		if (result != 4096) {break;}

		io.addr = (unsigned long) buffer;
		io.slba = LBA_BASE + pageCount1;

		set_start_time(&et);
		err = ioctl(fd, NVME_IOCTL_SUBMIT_IO, &io);
		add_exec_time(&et);
		if (err)
			fprintf(stderr, "nvme write status:%x\n", err);
		pageCount1++;
	}

	pageCount2 = pageCount1;
	while(1)
	{
		result = fread (buffer,1,4096,pFile2);
		if (result != 4096) {break;}

		io.addr = (unsigned long) buffer;
		io.slba = LBA_BASE + pageCount2;

		set_start_time(&et);
		err = ioctl(fd, NVME_IOCTL_SUBMIT_IO, &io);
		add_exec_time(&et);
		if (err)
			fprintf(stderr, "nvme write status:%x\n", err);
		pageCount2++;
	}


#if (VERBOSE)
	print_exec_time(&et);
	cout << "Page count: " << pageCount1 << ", " << pageCount2 << endl;
#endif
	cout << pageCount1 << ", " << pageCount2 << endl;

	fclose(pFile1);
	fclose(pFile2);
	free(buffer);

return 0;

//perror:
//	perror(perrstr);

return 1;
}
