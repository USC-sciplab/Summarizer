#include "nvme_tpch_query14.h"

extern exec_time_t acc_et_io, acc_et_proc;
extern unsigned int proc_delay;

void tpch_query14_ssd_all(char *date, int fd, void *buffer, unsigned int pageCount1, unsigned int pageCount2, double & promo_revenue, double & revenue)
{
	int err;
	struct nvme_user_io io;

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
	
	io.opcode = nvme_cmd_read;
	io.flags = 7;
	io.control = 0;
	io.metadata = (unsigned long) 0;
	io.addr = (unsigned long) buffer;
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

	for(unsigned int i2 = pageCount1; i2 < pageCount2; i2+=NUMBER_OF_PAGES_IN_A_COMMAND)
	{
		unsigned int pageInThisIteration2 = std::min((unsigned int)NUMBER_OF_PAGES_IN_A_COMMAND, (pageCount2-i2));
		io.slba = LBA_BASE + i2;
		io.nblocks = pageInThisIteration2-1;

		set_start_time(&et_io);
		err = ioctl(fd, NVME_IOCTL_SUBMIT_IO, &io);
		add_exec_time(&et_io);
		if (err)
			fprintf(stderr, "nvme read status:%x\n", err);
	}
	
	io.flags = 8;
	for(unsigned int i=0; i < pageCount1; i += NUMBER_OF_PAGES_IN_A_COMMAND)
	{
		unsigned int pageInThisIteration = std::min((unsigned int)NUMBER_OF_PAGES_IN_A_COMMAND, (pageCount1-i));
		io.slba = LBA_BASE + i;
		io.nblocks = pageInThisIteration-1;

		set_start_time(&et_io);
		err = ioctl(fd, NVME_IOCTL_SUBMIT_IO, &io);
		add_exec_time(&et_io);
		if (err)
			fprintf(stderr, "nvme read status:%x\n", err);
	}
	
	io.opcode = nvme_cmd_read;
	io.flags = 9;
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
	memcpy((char *)(&revenue), (char *)buffer, sizeof(double));
	memcpy((char *)(&promo_revenue), (((char *)buffer)+8), sizeof(double));
	
	print_exec_time(&et_io);
	print_exec_time(&et_proc);
	accumulate_exec_time(&acc_et_io, &et_io);
	accumulate_exec_time(&acc_et_proc, &et_proc);
	
	return;
}
