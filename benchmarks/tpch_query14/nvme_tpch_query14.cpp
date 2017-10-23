#include "nvme_tpch_query14.h"

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

bool compareDates(char *str1, char *str2, int interval)
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
		if(year2 < (year1)) return 1;
		else if(year2 > (year1)) return 0;
		else {
			if(month2 < (month1+interval)) return 1;
			else if(month2 > (month1+interval)) return 0;
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
	unsigned int pageCount1, pageCount2;
	int err, fd;
	struct nvme_user_io io;

	if (argc != 5) {
		fprintf(stderr, "Usage: %s <device> <pageCount1, 2> <proc_delay>\n", argv[0]);
		return 1;
	}

	perrstr = argv[1];
	pageCount1 = atoi(argv[2]);
	pageCount2 = atoi(argv[3]);
	proc_delay = atoi(argv[4]);
	fd = open(argv[1], O_RDWR);
	if (fd < 0)
	{
		printf("Error in commandline\n");
	//	goto perror;
	}
	if (pageCount1 > 0) {
		cout << "Page count1: " << pageCount1 << endl;
	} else {
		cout << "Page count1 should be bigger than 0" << endl;
		exit(1);
	}
	
	if (pageCount2 > 0) {
		cout << "Page count2: " << pageCount2 << endl;
	} else {
		cout << "Page count2 should be bigger than 0" << endl;
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
	QueryFilePtr = fopen( "query14_5.txt" , "r");
	if (QueryFilePtr==NULL) {fputs ("Query File error",stderr); exit (1);}
	unsigned int NumberOfQueries;
	fscanf(QueryFilePtr, "%u", &NumberOfQueries);
	char DateBuffer[50];
	double promo_revenue, revenue;

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
		promo_revenue = 0;
		revenue = 0;

		unsigned int temp;
		fscanf(QueryFilePtr, "%u %s\n", &temp, DateBuffer);
		cout << "==============================================================" << endl;
		set_start_time(&et);
	#if (SSD_PROC_MODE==0)	// baseline
		tpch_query14_baseline(DateBuffer, fd, buffer, pageCount1, pageCount2, promo_revenue, revenue);
	#endif

	#if (SSD_PROC_MODE==1)	// all processing at SSD
		tpch_query14_ssd_all(DateBuffer, fd, buffer, pageCount1, pageCount2, promo_revenue, revenue);
	#endif
	#if (SSD_PROC_MODE==2)	// comparison at SSD
		tpch_query14_ssd_cmp(DateBuffer, fd, buffer, pageCount1, pageCount2, promo_revenue, revenue);
	#endif
	#if (SSD_PROC_MODE==3)	// mixing of baseline + all at ssd
		tpch_query14_ssd_mix(DateBuffer, fd, buffer, pageCount1, pageCount2, promo_revenue, revenue);
	#endif

		add_exec_time(&et);

		//cout << pageCount << endl;
		cout << pageCount1 << " " << pageCount2 << " " <<  promo_revenue << " " << revenue << " " << (100 * promo_revenue ) / revenue << endl;
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
