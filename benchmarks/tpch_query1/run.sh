#!/bin/bash

source ../run.par

# store db on ssd
#make wr_db
#echo "sudo ./nvme_tpch_query1_wr_db /dev/nvme0n1"
#sudo ./nvme_tpch_query1_wr_db /dev/nvme0n1
#

# host processing
# query1 processing time on i5 = 80 us (usleep overhead = 57)
# org = 80, x2=103, x4=264, x8=583, x16=1223
PROC_DELAY="264"
#PROC_DELAY="0 103"
#PROC_DELAY="0 103 264 583"
#PROC_DELAY="0 103 264 583 1223"

N_PAGE=`cat pagecount.txt`
echo "${N_PAGE}"

if [ -f ${RES_FILE} ]; then
	echo "removing ${RES_FILE}"
	rm -f ${RES_FILE}
fi

touch ${RES_FILE}

# baseline
#make -f Makefile.base
# mix
make -f Makefile.mix

for i in ${PROC_DELAY}; do
	echo "*************************************" >> ${RES_FILE}
	echo "DELAY: ${i}" >> ${RES_FILE}
	echo "sudo ./nvme_tpch_query1 /dev/nvme0n1 ${N_PAGE} ${i} >> ${RES_FILE}"
	sudo ./nvme_tpch_query1 /dev/nvme0n1 ${N_PAGE} ${i} >> ${RES_FILE}
done

echo "done...."
