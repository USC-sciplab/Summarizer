#include "nvme_tpch_query14.h"

extern exec_time_t acc_et_io, acc_et_proc;
extern unsigned int proc_delay;

void tpch_query14_ssd_cmp(char *date, int fd, void *buffer, unsigned int pageCount1, unsigned int pageCount2, double & promo_revenue, double & revenue)
{
	int err;
	struct nvme_user_io io;

	char *p_type;
	
	std::unordered_map<unsigned long long int, char *> hashtable;

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

	// measure time
	exec_time_t et_io, et_proc;
	char et_name[64];
	strcpy(et_name, "io");
	init_exec_time(&et_io, et_name);
	strcpy(et_name, "proc");
	init_exec_time(&et_proc, et_name);
	
	int offset2=0;
	io.flags = 10;
	for(unsigned int i2 = pageCount1; i2 < pageCount2; i2+=NUMBER_OF_PAGES_IN_A_COMMAND)
	{
		unsigned int temp = 0;
		memcpy(buffer, (char *)(&temp), sizeof(unsigned int));

		unsigned int pageInThisIteration2 = std::min((unsigned int)NUMBER_OF_PAGES_IN_A_COMMAND, (pageCount2-i2));
		io.slba = LBA_BASE + i2;
		io.nblocks = pageInThisIteration2-1;
		
		set_start_time(&et_io);
		err = ioctl(fd, NVME_IOCTL_SUBMIT_IO, &io);
		add_exec_time(&et_io);
		if (err)
			fprintf(stderr, "nvme read status:%x\n", err);

		set_start_time(&et_proc);
		unsigned int NumOfRecords, j;
		memcpy((void *)(&NumOfRecords), (void *) buffer, sizeof(unsigned int));  
		int currentPage = 0, position = 0, currentPosition=0;
		unsigned long long int p_partkey;                  
		for(j = 0; j < NumOfRecords; j++)
		{      
			position = currentPosition; 
			if(currentPage == 0) position = currentPosition + 4;
            currentPosition += 13;                                           
            memcpy((void *)(&p_partkey), (void *)(((char *)buffer)+position+currentPage*4096), 8);
			p_type = (char *)calloc(1,6);
            memcpy((void *)(p_type), (void *)(((char *)buffer)+position+currentPage*4096+8), 5);
			hashtable.emplace(p_partkey, p_type);
            if((position + 13) >= 4096)                                      
            {       
				currentPage += 1; 
				currentPosition = 0;                                     
			}                                                                
		}
		add_exec_time(&et_proc);
	}
	
	int offset = 0;
	io.flags = 11;
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

		set_start_time(&et_proc);
		unsigned int NumOfRecords, j;
		memcpy((void *)(&NumOfRecords), (void *) buffer, sizeof(unsigned int));
		double l_discount, l_extendedprice;
		unsigned int l_partkey;
		int currentPage = 0, position = 0, currentPosition=0;
		for(j = 0; j < NumOfRecords; j++)
		{
			position = currentPosition;
			if(currentPage == 0) position = currentPosition + 4;
			currentPosition += 20;                                         

			memcpy((void *)(&l_partkey), (void *)(((char *)buffer)+position+currentPage*4096+0), 4);
			memcpy((void *)(&l_discount), (void *)(((char *)buffer)+position+currentPage*4096+4), 8);
			memcpy((void *)(&l_extendedprice), (void *)(((char *)buffer)+position+currentPage*4096+12), 8);
			
			std::unordered_map<unsigned long long int, char *>::iterator it;
			it = hashtable.find(l_partkey);
			if(it != hashtable.end())
			{
				if(strlen(it->second) >= 5)
				{
					if(strncmp(it->second, "PROMO",5) == 0)
					{
						promo_revenue += l_extendedprice * (1 - l_discount);
					}
				}
				revenue += l_extendedprice * (1 - l_discount);
			}

			if((position + 20) >= 4096)                                      
            {       
				currentPage += 1; 
				currentPosition = 0;                                     
             }                                                                
        }
		add_exec_time(&et_proc);
	}

	for ( auto local_it = hashtable.begin(); local_it != hashtable.end(); ++local_it )
		free(local_it->second);
	hashtable.clear();

	print_exec_time(&et_io);
	print_exec_time(&et_proc);
	accumulate_exec_time(&acc_et_io, &et_io);
	accumulate_exec_time(&acc_et_proc, &et_proc);
}
