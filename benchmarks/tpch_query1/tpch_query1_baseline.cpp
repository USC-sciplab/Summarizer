#include "nvme_tpch_query1.h"

extern exec_time_t acc_et_io, acc_et_proc;
extern unsigned int proc_delay;

void tpch_query1_baseline(char *date, int fd, void *buffer, unsigned int pageCount, std::map <std::string, class storage> &mymap)
{
	int err;
	struct nvme_user_io io;
	char *l_shipdate = (char *)calloc(9,1);
	double *l_discount = (double *)calloc(1,sizeof(double));
	double *l_quantitiy = (double *)calloc(1,sizeof(double));
	double *l_extendedprice = (double *)calloc(1,sizeof(double));
	double *l_tax = (double *) calloc(1,sizeof(double));
	std::string l_returnflagAndLinestatus;
	l_returnflagAndLinestatus.resize(2);

	std::map<char *, class storage>::iterator it;
	int offset = 0;
		
	io.opcode = nvme_cmd_read;
	io.flags = 0;
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

		for(unsigned int j = 0; j < pageInThisIteration; j++)
		{
			set_start_time(&et_proc);
			if (proc_delay > 0)
				usleep(proc_delay);

			while(1)
			{
				memcpy((char *)l_shipdate, (char *)(((char *)buffer)+4096*j+offset+50), 8);
			
				if((compareDates(date, l_shipdate, 0) == 1) or (strcmp(l_shipdate, date) == 0))
				{
					memcpy((char *)l_discount, (char *)(((char *)buffer)+4096*j+offset+32), 8);
					memcpy((char *)l_quantitiy, (char *)(((char *)buffer)+4096*j+offset+16), 8); 
					memcpy((char *)l_extendedprice, (char *)(((char *)buffer)+4096*j+offset+24), 8); 
					memcpy((char *)l_tax, (char *)(((char *)buffer)+4096*j+offset+40), 8);
					memcpy((char *)l_returnflagAndLinestatus.data(), (char *)(((char *)buffer)+4096*j+offset+48), 2);

					if(mymap.find(l_returnflagAndLinestatus) == mymap.end())
					{
						mymap[l_returnflagAndLinestatus];
						mymap[l_returnflagAndLinestatus].l_returnflag = l_returnflagAndLinestatus[0];
						mymap[l_returnflagAndLinestatus].l_linestatus = l_returnflagAndLinestatus[1];
						mymap[l_returnflagAndLinestatus].sum_qty = *l_quantitiy;
						mymap[l_returnflagAndLinestatus].sum_base_price = *l_extendedprice;
						mymap[l_returnflagAndLinestatus].sum_disc_price =  *l_extendedprice * (1 - *l_discount);
						mymap[l_returnflagAndLinestatus].sum_charge = *l_extendedprice * (1 - *l_discount) *(1 + *l_tax);
						mymap[l_returnflagAndLinestatus].avg_qty = *l_quantitiy;
						mymap[l_returnflagAndLinestatus].avg_price = *l_extendedprice;
						mymap[l_returnflagAndLinestatus].avg_disc = *l_discount;
						mymap[l_returnflagAndLinestatus].count_order = 1;
					}
					else {
						mymap[l_returnflagAndLinestatus];
						mymap[l_returnflagAndLinestatus].sum_qty += *l_quantitiy;
						mymap[l_returnflagAndLinestatus].sum_base_price += *l_extendedprice;
						mymap[l_returnflagAndLinestatus].sum_disc_price +=  (*l_extendedprice * (1 - *l_discount));
						mymap[l_returnflagAndLinestatus].sum_charge += (*l_extendedprice * (1 - *l_discount) *(1 + *l_tax));
						mymap[l_returnflagAndLinestatus].avg_qty += (*l_quantitiy);
						mymap[l_returnflagAndLinestatus].avg_price += (*l_extendedprice);
						mymap[l_returnflagAndLinestatus].avg_disc += *l_discount;
						mymap[l_returnflagAndLinestatus].count_order += 1;
					}
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

	print_exec_time(&et_io);
	print_exec_time(&et_proc);
	accumulate_exec_time(&acc_et_io, &et_io);
	accumulate_exec_time(&acc_et_proc, &et_proc);
	
	free(l_shipdate);
	free(l_discount);
	free(l_extendedprice);
	free(l_quantitiy);
	free(l_tax);
//	free(l_returnflagAndLinestatus);
}
