#!/bin/bash

source ../run.par

# store db on ssd
make wr_db
echo "sudo ./nvme_tpch_query1_wr_db /dev/nvme0n1"
sudo ./nvme_tpch_query1_wr_db /dev/nvme0n1
#

echo "done...."
