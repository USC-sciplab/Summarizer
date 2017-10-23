#include "nvme_tpch_query6.h"

extern exec_time_t acc_et_io, acc_et_proc;
extern unsigned int proc_delay;
extern unsigned int n_ssd_proc;

void tpch_query6_ssd_mix(char *date, double discount, unsigned int quantity, int fd, void *buffer, unsigned int pageCount, double &revenue)
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
	n_ssd_proc = 0;
		
	io.opcode = nvme_cmd_read;
	io.flags = 3;
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

	uint32_t *l_return_code = (uint32_t*)malloc( 1*sizeof(uint32_t) );

	for(unsigned int i=0; i < pageCount; i += NUMBER_OF_PAGES_IN_A_COMMAND)
	{
//		cout << "readPageCount = " << i << endl;
		unsigned int pageInThisIteration = std::min((unsigned int)NUMBER_OF_PAGES_IN_A_COMMAND, (pageCount-i));
		// copy the file into the buffer:
		//		result = fread (buffer,1,4096,pFile);
		//		if (result != 4096) {break;}
		io.flags = 3;
		io.slba = LBA_BASE + i;
		io.nblocks = pageInThisIteration-1;

		set_start_time(&et_io);
		err = ioctl(fd, NVME_IOCTL_SUBMIT_IO, &io);
		add_exec_time(&et_io);
		if (err)
			fprintf(stderr, "nvme read status:%x\n", err);

		for (unsigned int j = 0; j < pageInThisIteration; j++) {
			memcpy( (uint8_t*)l_return_code, (uint8_t*)( (uint8_t*)buffer + 4096*j ), sizeof(uint32_t) );
			if ( *l_return_code==(uint32_t)-1 ) {
				//printf("l_return_code: %u\n", *l_return_code);
				n_ssd_proc++;
				continue;
			}
			// CPU is working
			set_start_time(&et_proc);
			if (proc_delay > 0)
				usleep(proc_delay);

			while(1)
			{
				memcpy((char *)l_shipdate, (char *)(((char *)buffer)+4096*j+offset+50), 8);
				memcpy((char *)l_discount, (char *)(((char *)buffer)+4096*j+offset+32), 8);
				memcpy((char *)l_quantitiy, (char *)(((char *)buffer)+4096*j+offset+16), 8); 

				//			cout << compareDates(l_shipdate, "19940101", 0) << " " << compareDates("19940101", l_shipdate, 1) << " " << strcmp(l_shipdate, "19940101") << endl;
				if( (((compareDates(l_shipdate, date, 0) == 1) and (compareDates(date, l_shipdate, 1) == 1)) or (strcmp(l_shipdate, date) == 0)) and (*l_discount > (discount - 0.01) ) and (*l_discount < (discount + 0.01)) and (*l_quantitiy < quantity))
				{
					//			cout << "Discount comparison is " << ((*l_discount > (0.06 - 0.01) )) << " " << ((*l_discount < (0.06 + 0.01))) << endl;
					memcpy((char *)l_extendedprice, (char *)(((char *)buffer)+4096*j+offset+24), 8);
					revenue += *l_extendedprice * *l_discount;
					//			cout << "Revenue is = " << revenue << endl;
				}

				offset += 153;
				if((offset + 153) >= (4096) )
				{
					offset = 0;
					break;
				}
			}
			add_exec_time(&et_proc);
		}
	}

	/*This code is to get the final result stored at the nvme controller. We set flags variable to 5.
	 */

	io.opcode = nvme_cmd_read;
	io.flags = 5;
	io.control = 0;
	io.metadata = (unsigned long) 0;
	io.addr = (unsigned long) buffer;
	io.slba = LBA_BASE;
	io.nblocks = 0;
	io.dsmgmt = 0;
	io.reftag = 0;
	io.apptag = 0;
	io.appmask = 0;

	//set_start_time(&et_proc);
	double revenue_ssd;
	err = ioctl(fd, NVME_IOCTL_SUBMIT_IO, &io);
	if (err)
		fprintf(stderr, "nvme read status:%x\n", err);

	memcpy((void *)&revenue_ssd, (void *)(buffer), 8);
	revenue += revenue_ssd;
	//add_exec_time(&et_proc);

	printf("Number of pages processed in SSD: %u\n", n_ssd_proc);
	print_exec_time(&et_io);
	print_exec_time(&et_proc);
	accumulate_exec_time(&acc_et_io, &et_io);
	accumulate_exec_time(&acc_et_proc, &et_proc);
}
