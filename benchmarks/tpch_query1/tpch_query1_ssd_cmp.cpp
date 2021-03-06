#include "nvme_tpch_query1.h"

extern exec_time_t acc_et_io, acc_et_proc;
extern unsigned int proc_delay;

void tpch_query1_ssd_cmp(char *date, int fd, void *buffer, unsigned int pageCount, std::map <std::string, class storage> &mymap)
{
	int err;
	struct nvme_user_io io;

	double l_discount; 
	double l_quantitiy; 
	double l_extendedprice; 
	double l_tax; 
	std::string l_returnflagAndLinestatus;
	l_returnflagAndLinestatus.resize(2);

	memcpy((char *)buffer, date, 9);

	//Need to change flags
	io.opcode = nvme_cmd_read;
	io.flags = 4;
	io.control = 0;
	io.metadata = (unsigned long) 0;
	io.addr = (unsigned long) buffer;
	io.slba = LBA_BASE;
	io.nblocks = 0;
	io.dsmgmt = 0;
	io.reftag = 0;
	io.apptag = 0;
	io.appmask = 0;

	err = ioctl(fd, NVME_IOCTL_SUBMIT_IO, &io);
	if (err)
		fprintf(stderr, "nvme read status:%x\n", err);

	// in-ssd processing
	io.opcode = nvme_cmd_read;
	io.flags = 14;
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
        memcpy(buffer, (char *)(&temp), sizeof(unsigned int));

		unsigned int pageInThisIteration = std::min((unsigned int)NUMBER_OF_PAGES_IN_A_COMMAND, (pageCount-i));
		// copy the file into the buffer:
		//		result = fread (buffer,1,4096,pFile);
		//		if (result != 4096) {break;}
		io.slba = LBA_BASE + i;
		io.nblocks = pageInThisIteration-1;

		set_start_time(&et_io);
		err = ioctl(fd, NVME_IOCTL_SUBMIT_IO, &io);
		add_exec_time(&et_io);
		if (err)
			fprintf(stderr, "nvme read status:%x\n", err);

		set_start_time(&et_proc);
		unsigned int NumOfRecords, j;
		memcpy((void *)(&NumOfRecords), (void *) buffer, sizeof(unsigned int));
		int currentPage = 0, position = 0, currentPosition=0;
		for(j = 0; j < NumOfRecords; j++)
		{
			position = currentPosition;
			if(currentPage == 0) position = currentPosition + 4;
			currentPosition += 34;
			memcpy((char *)(l_returnflagAndLinestatus.data()), (void *)(((char *)buffer)+position+currentPage*4096), 2);
			memcpy((char *)(&l_discount), (void *)(((char *)buffer)+position+currentPage*4096+2), 8);
			memcpy((char *)(&l_quantitiy), (void *)(((char *)buffer)+position+currentPage*4096+10), 8);
			memcpy((char *)(&l_extendedprice), (void *)(((char *)buffer)+position+currentPage*4096+18), 8);
			memcpy((char *)(&l_tax), (void *)(((char *)buffer)+position+currentPage*4096+26), 8);

			if(mymap.find(l_returnflagAndLinestatus) == mymap.end())
			{
				mymap[l_returnflagAndLinestatus];
				mymap[l_returnflagAndLinestatus].l_returnflag = l_returnflagAndLinestatus[0];
				mymap[l_returnflagAndLinestatus].l_linestatus = l_returnflagAndLinestatus[1];
				mymap[l_returnflagAndLinestatus].sum_qty = l_quantitiy;
				mymap[l_returnflagAndLinestatus].sum_base_price = l_extendedprice;
				mymap[l_returnflagAndLinestatus].sum_disc_price =  l_extendedprice * (1 - l_discount);
				mymap[l_returnflagAndLinestatus].sum_charge = l_extendedprice * (1 - l_discount) *(1 + l_tax);
				mymap[l_returnflagAndLinestatus].avg_qty = l_quantitiy;
				mymap[l_returnflagAndLinestatus].avg_price = l_extendedprice;
				mymap[l_returnflagAndLinestatus].avg_disc = l_discount;
				mymap[l_returnflagAndLinestatus].count_order = 1;
			}
			else {
				mymap[l_returnflagAndLinestatus];
				mymap[l_returnflagAndLinestatus].sum_qty += l_quantitiy;
				mymap[l_returnflagAndLinestatus].sum_base_price += l_extendedprice;
				mymap[l_returnflagAndLinestatus].sum_disc_price +=  (l_extendedprice * (1 - l_discount));
				mymap[l_returnflagAndLinestatus].sum_charge += (l_extendedprice * (1 - l_discount) *(1 + l_tax));
				mymap[l_returnflagAndLinestatus].avg_qty += (l_quantitiy);
				mymap[l_returnflagAndLinestatus].avg_price += (l_extendedprice);
				mymap[l_returnflagAndLinestatus].avg_disc += l_discount;
				mymap[l_returnflagAndLinestatus].count_order += 1;
			}

			if((position + 34) >= 4096)
			{
				currentPage += 1;
				currentPosition = 0;
			}
		}
		add_exec_time(&et_proc);
	}
	
	print_exec_time(&et_io);
	print_exec_time(&et_proc);
	accumulate_exec_time(&acc_et_io, &et_io);
	accumulate_exec_time(&acc_et_proc, &et_proc);
}
