#include "nvme_similarity_join.h"

void cmpAllAtSSD(int fd, void *buffer, unsigned int pageCount, record queryRecord)
{
	int err;
	struct nvme_user_io io;

	/*Generate commands to read the block, keeping flags variable as 3. These commands will be intercepted by the nvme controller and then processed by it. Nvme controller at the arm core processes the data it gets from the storage and keeps the result at the arm core.*/
	/*Things to do: 1) Experiment with various read lengths */

	revenue = 0;
	int offset = 0;
	for(unsigned int i=0; i < pageCount; i += NUMBER_OF_PAGES_IN_A_COMMAND)
	{
		unsigned int temp = 0;
		memcpy((void *) buffer, (void *)(&temp), sizeof(unsigned int));
//		cout << "readPageCount = " << i << endl;
		unsigned int pageInThisIteration = std::min((unsigned int)NUMBER_OF_PAGES_IN_A_COMMAND, (pageCount-i));
		// copy the file into the buffer:
		//		result = fread (buffer,1,4096,pFile);
		//		if (result != 4096) {break;}
		io.opcode = nvme_cmd_read;
		//		io.flags = 0;
		io.flags = 6;
		io.control = 0;
		io.metadata = (unsigned long) 0;
		io.addr = (unsigned long) buffer;
		io.slba = i;
		io.nblocks = pageInThisIteration-1;
		io.dsmgmt = 0;
		io.reftag = 0;
		io.apptag = 0;
		io.appmask = 0;

		err = ioctl(fd, NVME_IOCTL_SUBMIT_IO, &io);
		//		if (err < 0)
		//			goto perror;
		unsigned int NumOfRecords, j;
		memcpy((void *)(&NumOfRecords), (void *) buffer, sizeof(unsigned int));
		int currentPage = 0, position = 0,currentPosition=0;
		double l_extendedPrice, l_discount;
		for(j = 0; j < NumOfRecords; j++)
		{
			position = currentPosition;
			if(currentPage == 0) position = currentPosition + 4;
			currentPosition += 16;
			memcpy((void *)(&l_extendedPrice), (void *)(((char *)buffer)+position+currentPage*4096), 8);
			memcpy((void *)(&l_discount), (void *)(((char *)buffer)+position+currentPage*4096+8), 8);
			revenue += l_extendedPrice * l_discount;
			if((position + 16) >= 4096)
			{
				currentPage += 1;
				currentPosition = 0;
			}
		}
		if (err)
			fprintf(stderr, "nvme read status:%x\n", err);
	}
}
