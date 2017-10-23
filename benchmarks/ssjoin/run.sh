#!/bin/bash

source ../run.par

# store db on ssd
#make wr_db
#echo "sudo ./nvme_ssjoin_wr_db /dev/nvme0n1"
#sudo ./nvme_ssjoin_wr_db /dev/nvme0n1
#

# host processing
# baseline processing time = 95 us / page
# usleep overhead = 57 us
PROC_DELAY="38"
#PROC_DELAY="0 38"

N_PAGE=`cat pagecount.txt`
echo "${N_PAGE}"

if [ -f ${RES_FILE} ]; then
	echo "removing ${RES_FILE}"
	rm -f ${RES_FILE}
fi

touch ${RES_FILE}

# baseline
make -f Makefile.base
# mix
#make -f Makefile.mix

for i in ${PROC_DELAY}; do
	echo "*************************************" >> ${RES_FILE}
	echo "DELAY: ${i}" >> ${RES_FILE}
	echo "sudo ./nvme_ssjoin /dev/nvme0n1 ${N_PAGE} ${i} >> ${RES_FILE}"
	sudo ./nvme_ssjoin /dev/nvme0n1 ${N_PAGE} ${i} >> ${RES_FILE}
done

echo "done...."
