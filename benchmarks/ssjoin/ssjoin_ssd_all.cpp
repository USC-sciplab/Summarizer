#include "nvme_ssjoin.h"

extern exec_time_t acc_et_io, acc_et_proc;
extern unsigned int proc_delay;

void ssjoin_ssd_all(int fd, void *buffer, unsigned int pageCount)
{
	int err;
	struct nvme_user_io io;
	/*Generate commands to read the block, keeping flags variable as 3. These commands will be intercepted by the nvme controller and then processed by it. Nvme controller at the arm core processes the data it gets from the storage and keeps the result at the arm core.*/
	/*Things to do: 1) Experiment with various read lengths */

	int numRecords = 0;
	int offset = 0;

	exec_time_t et_io, et_proc;
	char et_name[64];
	strcpy(et_name, "io");
	init_exec_time(&et_io, et_name);
	strcpy(et_name, "proc");
	init_exec_time(&et_proc, et_name);

	for(unsigned int i=0; i < pageCount; i += NUMBER_OF_PAGES_IN_A_COMMAND)
	{
//		cout << "readPageCount = " << i << endl;
		unsigned int pageInThisIteration = std::min((unsigned int)NUMBER_OF_PAGES_IN_A_COMMAND, (pageCount-i));
		// copy the file into the buffer:
		//		result = fread (buffer,1,4096,pFile);
		//		if (result != 4096) {break;}
		io.opcode = nvme_cmd_read;
		//		io.flags = 0;
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
		//		break;
	}

	/*This code is to get the final result stored at the nvme controller. We set flags variable to 5.
	 */

	io.opcode = nvme_cmd_read;
	io.flags = 20;
	io.control = 0;
	io.metadata = (unsigned long) 0;
	io.addr = (unsigned long) buffer;
	io.slba = LBA_BASE+0;
	io.nblocks = 0;
	io.dsmgmt = 0;
	io.reftag = 0;
	io.apptag = 0;
	io.appmask = 0;

	err = ioctl(fd, NVME_IOCTL_SUBMIT_IO, &io);
	//            if (err < 0)
	//                      goto perror;
	if (err)
		fprintf(stderr, "nvme read status:%x\n", err);
    //Similar
	set_start_time(&et_proc);
	memcpy((void *)&numRecords, (void *)(buffer), 4);
	add_exec_time(&et_proc);
	cout << "Verified records " << numRecords << endl;

	print_exec_time(&et_io);
	print_exec_time(&et_proc);
	accumulate_exec_time(&acc_et_io, &et_io);
	accumulate_exec_time(&acc_et_proc, &et_proc);
}
