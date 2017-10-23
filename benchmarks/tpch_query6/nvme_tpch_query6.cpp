#include "nvme_tpch_query6.h"

// these functions are used for baseline
void convert(char *date, int &year, int &month, int &day)
{
	char *Year = (char *)calloc(5,1);
	memcpy((void *)Year, (void *)date, 4);
	year = atoi(Year);
	char *Month = (char *)calloc(3,1);
	memcpy((void *)Month, (void *)(date+4), 2);
	month = atoi(Month);
	char *Day = (char *)calloc(3,1);
	memcpy((void *)Day, (void *)(date+6), 2);
	day = atoi(Day);
}

bool compareDates(char *str1, char *str2, int interval) // When interval is 0, this return 1 if the str2 date is less than str1 date
{
	int year1, year2, month1, month2, day1, day2;
	convert(str1, year1, month1, day1);
	convert(str2, year2, month2, day2);
	if(interval == 0)
	{
		if(year2 < year1) return 1;
		else if(year2 > year1) return 0;
		else {
			if(month2 < month1) return 1;
			else if(month2 > month1) return 0;
			else {
				if(day2 < day1) return 1;
				else if(day2 > day1) return 0;
				else {
					return 0;
				}
			}
		}
	}
	else {
		if(year2 < (year1+interval)) return 1;
		else if(year2 > (year1+interval)) return 0;
		else {
			if(month2 < month1) return 1;
			else if(month2 > month1) return 0;
			else {
				if(day2 < day1) return 1;
				else if(day2 > day1) return 0;
				else {
					return 0;
				}
			}
		}
	}
}

exec_time_t acc_et_io, acc_et_proc;
unsigned int proc_delay;
unsigned int n_ssd_proc;

int main(int argc, char **argv)
{
	static const char *perrstr;
	unsigned int pageCount;
	int err, fd;
	struct nvme_user_io io;

	if (argc != 4) {
		fprintf(stderr, "Usage: %s <device> <pageCount> <proc_delay>\n", argv[0]);
		return 1;
	}

	perrstr = argv[1];
	pageCount = atoi(argv[2]);
	proc_delay = 0;
	proc_delay = atoi(argv[3]);
	fd = open(argv[1], O_RDWR);
	if (fd < 0)
	{
		printf("Error in commandline\n");
	//	goto perror;
	}
	if (pageCount > 0) {
		cout << "Page count: " << pageCount << endl;
	} else {
		cout << "Page count should be bigger than 0" << endl;
		exit(1);
	}
	n_ssd_proc = 0;

	// DB data should be stored in SSD

	void *buffer;
	
	if (posix_memalign(&buffer, 4096, REQUEST_SIZE)) {
		fprintf(stderr, "can not allocate io payload\n");
		return 0;
	}

	/*Read the values written and process each block by block*/
	/*Things to do: 1) Experiment with various read lengths */

	FILE *QueryFilePtr;
	QueryFilePtr = fopen( "query6_5.txt" , "r");
	if (QueryFilePtr==NULL) {fputs ("Query File error",stderr); exit (1);}
	unsigned int NumberOfQueries;
	fscanf(QueryFilePtr, "%u", &NumberOfQueries);
	char date[20];
	double discount;
	unsigned int quantity;

	cout << "Number of queries = " << NumberOfQueries << endl;

	// io command (basic)
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
		fscanf(QueryFilePtr, "%u %s %lf %u\n", &temp, date, &discount, &quantity);
		cout << "==============================================================" << endl;
		double revenue = 0;
		set_start_time(&et);
	#if (SSD_PROC_MODE==0)	// baseline
		tpch_query6_baseline(date, discount, quantity, fd, buffer, pageCount, revenue);
	#endif

	#if (SSD_PROC_MODE > 0)
		// Processing is performend in SSD, io.flags=4 is initialization command
		memcpy((char *)buffer, date, 9);
		memcpy(((char *)buffer)+9, (char *)&discount, 8);
		memcpy(((char *)buffer)+17, (char *)&quantity, 4);
		io.flags = 4;
		io.slba = LBA_BASE + pageCount;

		err = ioctl(fd, NVME_IOCTL_SUBMIT_IO, &io);
		if (err)
			fprintf(stderr, "nvme read status:%x\n", err);
	#endif // (SSD_PROC_MODE > 0)
	#if (SSD_PROC_MODE==1)	// all processing at SSD
		tpch_query6_ssd_all(date, discount, quantity, fd, buffer, pageCount, revenue);
	#endif
	#if (SSD_PROC_MODE==2)	// comparison at SSD
		tpch_query6_ssd_cmp(date, discount, quantity, fd, buffer, pageCount, revenue);
	#endif
	#if (SSD_PROC_MODE==3)	// mixing of baseline + all at ssd
		tpch_query6_ssd_mix(date, discount, quantity, fd, buffer, pageCount, revenue);
	#endif

		add_exec_time(&et);
		//cout << pageCount << endl;
		cout << IterationForQuery << " revenue = " << revenue << endl;
		cout << "==============================================================" << endl;
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
