#include "nvme_tpch_query6.h"

extern exec_time_t acc_et_io, acc_et_proc;
extern unsigned int proc_delay;

void tpch_query6_ssd_cmp(char *date, double discount, unsigned int quantity, int fd, void *buffer, unsigned int pageCount, double &revenue)
{
	int err;
	struct nvme_user_io io;

	char *l_shipdate = (char *)calloc(9,1);
	double *l_discount = (double *)calloc(1,sizeof(double));
	double *l_quantitiy = (double *)calloc(1,sizeof(double));
	double *l_extendedprice = (double *)calloc(1,sizeof(double));

	/*Generate commands to read the block, keeping flags variable as 3. These commands will be intercepted by the nvme controller and then processed by it. Nvme controller at the arm core processes the data it gets from the storage and keeps the result at the arm core.*/
	/*Things to do: 1) Experiment with various read lengths */

	revenue = 0;
	int offset = 0;
	
	io.opcode = nvme_cmd_read;
	io.flags = 6;
	io.control = 0;
	io.metadata = (unsigned long) 0;
	io.addr = (unsigned long) buffer;
	io.slba = LBA_BASE;
	io.nblocks = 0;
	io.dsmgmt = 0;
	io.reftag = 0;
	io.apptag = 0;
	io.appmask = 0;
	
	// measure time
	exec_time_t et_io, et_proc;
	char et_name[64];
	strcpy(et_name, "io");
	init_exec_time(&et_io, et_name);
	strcpy(et_name, "proc");
	init_exec_time(&et_proc, et_name);

	for(unsigned int i=0; i < pageCount; i += NUMBER_OF_PAGES_IN_A_COMMAND)
	{
		unsigned int temp = 0;
		memcpy((void *) buffer, (void *)(&temp), sizeof(unsigned int));
//		cout << "readPageCount = " << i << endl;
		unsigned int pageInThisIteration = std::min((unsigned int)NUMBER_OF_PAGES_IN_A_COMMAND, (pageCount-i));
		// copy the file into the buffer:
		//		result = fread (buffer,1,4096,pFile);
		//		if (result != 4096) {break;}
		io.flags = 6;
		io.slba = LBA_BASE + i;
		io.nblocks = pageInThisIteration-1;

		set_start_time(&et_io);
		err = ioctl(fd, NVME_IOCTL_SUBMIT_IO, &io);
		add_exec_time(&et_io);
	
		set_start_time(&et_proc);
		unsigned int NumOfRecords, j;
		memcpy((void *)(&NumOfRecords), (void *) buffer, sizeof(unsigned int));
		int currentPage = 0, position = 0,currentPosition=0;
		double l_extendedPrice, l_discount;
		for(j = 0; j < NumOfRecords; j++)
		{
			currentPosition += 16;
			if(currentPage == 0) position = currentPosition + 4;
			memcpy((void *)(&l_extendedPrice), (void *)(((char *)buffer)+position+currentPage*4096), 8);
			memcpy((void *)(&l_discount), (void *)(((char *)buffer)+position+currentPage*4096+8), 8);
			revenue += l_extendedPrice * l_discount;
			if((position + 16) >= 4096)
			{
				currentPage += 1;
				currentPosition = 0;
			}
		}
		add_exec_time(&et_proc);
		if (err)
			fprintf(stderr, "nvme read status:%x\n", err);
	}

	print_exec_time(&et_io);
	print_exec_time(&et_proc);
	accumulate_exec_time(&acc_et_io, &et_io);
	accumulate_exec_time(&acc_et_proc, &et_proc);
}
