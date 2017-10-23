#include "nvme_tpch_query1.h"

extern exec_time_t acc_et_io, acc_et_proc;
extern unsigned int proc_delay;

void tpch_query1_ssd_all(char *date, int fd, void *buffer, unsigned int pageCount, std::map <std::string, class storage> &mymap)
{
	int err;
	struct nvme_user_io io;

	memcpy((char *)buffer, date, 9);

	// for initialization in ssd
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
	io.flags = 12;
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
	}

	// finalize in ssd
	unsigned int temp=0;
	memcpy(buffer, (char *)&temp, sizeof(unsigned int));

	io.opcode = nvme_cmd_read;
	io.flags = 13;
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

	// final processing
	set_start_time(&et_proc);
	class storage result;
	unsigned int NumOfRecords, j;
	memcpy((void *)(&NumOfRecords), (void *) buffer, sizeof(unsigned int));
	int currentPage = 0, position = 0, currentPosition=0;
	unsigned long long int p_partkey;
	for(j = 0; j < NumOfRecords; j++)
	{
		position = currentPosition;
		if(currentPage == 0) position = currentPosition + 4;
		currentPosition += 68;
		memcpy((void *)(&result.l_returnflag), (void *)(((char *)buffer)+position+currentPage*4096+2+0), 1);
		memcpy((void *)(&result.l_linestatus), (void *)(((char *)buffer)+position+currentPage*4096+2+1), 1);
		std::string temp;
		temp.resize(2);
		temp[0] = result.l_returnflag;
		temp[1] = result.l_linestatus;
		memcpy((void *)(&result.sum_qty), (void *)(((char *)buffer)+position+currentPage*4096+2+2), 8);
		memcpy((void *)(&result.sum_base_price), (void *)(((char *)buffer)+position+currentPage*4096+2+10), 8);
		memcpy((void *)(&result.sum_disc_price), (void *)(((char *)buffer)+position+currentPage*4096+2+18), 8);
		memcpy((void *)(&result.sum_charge), (void *)(((char *)buffer)+position+currentPage*4096+2+26), 8);
		memcpy((void *)(&result.avg_qty), (void *)(((char *)buffer)+position+currentPage*4096+2+34), 8);
		memcpy((void *)(&result.avg_price), (void *)(((char *)buffer)+position+currentPage*4096+2+42), 8);
		memcpy((void *)(&result.avg_disc), (void *)(((char *)buffer)+position+currentPage*4096+2+50), 8);
		memcpy((void *)(&result.count_order), (void *)(((char *)buffer)+position+currentPage*4096+2+58), 8);
		mymap[temp];
		mymap[temp] = result;
//		cout << result.l_returnflag << " " << result.l_linestatus << " " << result.sum_qty <<  " " << result.sum_base_price <<  " " << result.sum_disc_price <<  " " << result.sum_charge <<  " " << result.avg_qty <<  " " << result.avg_price <<  " " << result.avg_disc <<  " " << result.count_order << endl;  
		if((position + 68) >= 4096)
		{
			currentPage += 1;
			currentPosition = 0;
		}
	}
	add_exec_time(&et_proc);
	
	print_exec_time(&et_io);
	print_exec_time(&et_proc);
	accumulate_exec_time(&acc_et_io, &et_io);
	accumulate_exec_time(&acc_et_proc, &et_proc);
}
