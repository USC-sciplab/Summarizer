#!/bin/bash

source ../run.par

# store db on ssd
#make wr_db
#echo "sudo ./nvme_tpch_query6_wr_db /dev/nvme0n1"
#sudo ./nvme_tpch_query6_wr_db /dev/nvme0n1
#

# host processing
PROC_DELAY="71"
#PROC_DELAY="0 71 199"
#PROC_DELAY="0 7"
#PROC_DELAY="0 7 71 199"
#PROC_DELAY="0 7 71 199 455"
#PROC_DELAY="0 7 71 199 455 967"

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
	echo "sudo ./nvme_tpch_query6 /dev/nvme0n1 ${N_PAGE} ${i} >> ${RES_FILE}"
	sudo ./nvme_tpch_query6 /dev/nvme0n1 ${N_PAGE} ${i} >> ${RES_FILE}
done

echo "done...."
#sudo ./nvme_tpch_query6 /dev/nvme0n1 ${N_PAGE}
#sudo ./nvme_tpch_query6 /dev/sda3 ${N_PAGE}
