# Makefile for nvme controller (C/C++ mix)

# Environments
rdp = /mnt/ram_disk
sz = 1024
TARGET = issd_nvme
###########################

CFLAGS := -pthread -lrt -lm
#CFLAGS := -pthread -pg -g -lrt -lm
CFLAGS += -D_GNU_SOURCE
CFLAGS += -Wno-packed-bitfield-compat
CFLAGS += -I. -I./bdbm_ftl -I./bdbm_ftl/ftl

SRC_NVME = cli_parser.c \
           cmd_defn.c \
           common.c \
           dlcl.c \
           dma_df_nand.c \
           dma_mgr.c \
           i2c_dimm.c \
           nvme_bio.c \
           nvme.c \
           smbus.c \
           telnetd.c \
		   util.c

SRC_FTL  = bdbm_ftl/params.c \
           bdbm_ftl/platform.c \
           bdbm_ftl/ioctl.c \
           bdbm_ftl/kmain.c \
           bdbm_ftl/hlm_nobuf.c \
           bdbm_ftl/host_block.c \
           bdbm_ftl/llm_noq.c \
           bdbm_ftl/dm_dragonfire.c \
           bdbm_ftl/dev_dragonfire.c

SRC_FTL2 = bdbm_ftl/ftl/abm.c \
           bdbm_ftl/ftl/page_ftl.c

SRC_PROC = in_proc/add_buf10.cpp \
	in_proc/tpch_query14.cpp \
	in_proc/tpch_query14_cmp_all.cpp \
	in_proc/tpch_query1.cpp \
	in_proc/tpch_query1_cmp_all.cpp \
	in_proc/afc_sticky.cpp \
	in_proc/join.cpp

OBJ      = cli_parser.o \
           cmd_defn.o \
           common.o \
           dlcl.o \
           dma_df_nand.o \
           dma_mgr.o \
           i2c_dimm.o \
           nvme_bio.o \
           nvme.o \
           smbus.o \
           telnetd.o \
           params.o \
           platform.o \
           ioctl.o \
           kmain.o \
           hlm_nobuf.o \
           host_block.o \
           llm_noq.o \
           dm_dragonfire.o \
           dev_dragonfire.o \
           abm.o \
           page_ftl.o \
		   util.o \
	   add_buf10.o \
	   tpch_query14.o \
	   tpch_query14_cmp_all.o \
	   tpch_query1.o \
	   tpch_query1_cmp_all.o \
	   afc_sticky.o \
	   join.o

#############################################
# Custom defines
CFLAGS += -DVERBOSE=0
CFLAGS += -DGK_TEST_RD_NAND=3
CFLAGS += -DIN_PROC_RATE=0
CFLAGS += -DGK_REQ_TYPE=0
# DMA delay for LS2-HOST bw control
# x1=91, x2=260, x3=429, x4=598, x8=1274
# revised: x1=90, x2=275, x3=454, x4=625
CFLAGS += -DGK_DMA_DELAY=0
#CFLAGS += -DGK_DMA_DELAY=90
#CFLAGS += -DGK_DMA_DELAY=275
#CFLAGS += -DGK_DMA_DELAY=454
#CFLAGS += -DGK_DMA_DELAY=625
#CFLAGS += -DGK_DMA_DELAY=1274
###
# in-proc delay (usleep overhead = 55 us)
# query6, org=70, x2=15, x4=225, x8=505, x16=1065, x32=2185, x48=3235
CFLAGS += -DGK_PROC_DELAY_QUERY6=0
#CFLAGS += -DGK_PROC_DELAY_QUERY6=15
#CFLAGS += -DGK_PROC_DELAY_QUERY6=225
#CFLAGS += -DGK_PROC_DELAY_QUERY6=505
#CFLAGS += -DGK_PROC_DELAY_QUERY6=1065
#CFLAGS += -DGK_PROC_DELAY_QUERY6=2185
#CFLAGS += -DGK_PROC_DELAY_QUERY6=3235
###
# in-proc delay (usleep overhead = 55 us)
# query1, org=168, x2=281, x4=617, x8=1289, x16=2633, x32=5852, x48=7841
CFLAGS += -DGK_PROC_DELAY_QUERY1=0
#CFLAGS += -DGK_PROC_DELAY_QUERY1=168
#CFLAGS += -DGK_PROC_DELAY_QUERY1=281
#CFLAGS += -DGK_PROC_DELAY_QUERY1=617
#CFLAGS += -DGK_PROC_DELAY_QUERY1=1289
#CFLAGS += -DGK_PROC_DELAY_QUERY1=2633
#CFLAGS += -DGK_PROC_DELAY_QUERY1=5852
#CFLAGS += -DGK_PROC_DELAY_QUERY1=7841
###
# in-proc delay (usleep overhead = 55 us)
# query14, org=52, x2=49, x4=153, x8=361, x16=777, x32=1609, x48=2530
CFLAGS += -DGK_PROC_DELAY_QUERY14=0
#CFLAGS += -DGK_PROC_DELAY_QUERY14=49
#CFLAGS += -DGK_PROC_DELAY_QUERY14=153
#CFLAGS += -DGK_PROC_DELAY_QUERY14=361
#CFLAGS += -DGK_PROC_DELAY_QUERY14=777
#CFLAGS += -DGK_PROC_DELAY_QUERY14=1609
#CFLAGS += -DGK_PROC_DELAY_QUERY14=2530
#CFLAGS += -DGK_PROC_DELAY_PREFIX_SIMILARITY = 0
####
#CFLAGS += -DGK_FUNC_DELAY=245
#CFLAGS += -DGK_FUNC_DELAY=6974
CFLAGS += -DGK_TEST_RD_DUMMY=0
CFLAGS += -DGK_MEASURE_TIME=0
cFLAGS += -DGK_SPLIT_HOST_TX=0
CFLAGS += -DKM_TEST_RD_NAND=0

CFLAGS += -DKM_TPCH_QUERY6=0
CFLAGS += -DKM_TPCH_QUERY6_CMP_ALL=0
CFLAGS += -DKM_TPCH_QUERY14=0
CFLAGS += -DKM_TPCH_QUERY14_CMP_ALL=0
CFLAGS += -DKM_TPCH_QUERY1=0
CFLAGS += -DKM_TPCH_QUERY1_CMP_ALL=0
CFLAGS += -DKM_TEST_TRANSFER_TIME=1

CFLAGS += -DTE_AFC_STICKY=1
CFLAGS += -DHV_PREFIX_SIMILARITY=1
CFLAGS += -DHV_PREFIX_SIMILARITY_CMP_ALL=0
############################################

# for fsluissd
CFLAGS += -DGET_CMD_STATUS
CFLAGS += -DCUR_SETUP=TARGET_SETUP
CFLAGS += -DBIO_MEMTYPE=NVME_BIO_TWO

all: fsluissd

obj_nvme:
	$(CC) -c -o cli_parser.o  cli_parser.c  $(CFLAGS)
	$(CC) -c -o cmd_defn.o    cmd_defn.c    $(CFLAGS)
	$(CC) -c -o common.o      common.c      $(CFLAGS)
	$(CC) -c -o dlcl.o        dlcl.c        $(CFLAGS)
	$(CC) -c -o dma_mgr.o     dma_mgr.c     $(CFLAGS)
	$(CC) -c -o i2c_dimm.o    i2c_dimm.c    $(CFLAGS)
	$(CC) -c -o nvme_bio.o    nvme_bio.c    $(CFLAGS)
	$(CC) -c -o nvme.o        nvme.c        $(CFLAGS)
	$(CC) -c -o smbus.o       smbus.c       $(CFLAGS)
	$(CC) -c -o telnetd.o     telnetd.c     $(CFLAGS)
	$(CC) -c -o dma_df_nand.o dma_df_nand.c $(CFLAGS)
	$(CC) -c -o util.o        util.c        $(CFLAGS)

obj_ftl:
	$(CC) -c -o params.o         bdbm_ftl/params.c          $(CFLAGS)
	$(CC) -c -o platform.o       bdbm_ftl/platform.c        $(CFLAGS)
	$(CC) -c -o ioctl.o          bdbm_ftl/ioctl.c           $(CFLAGS)
	$(CC) -c -o kmain.o          bdbm_ftl/kmain.c           $(CFLAGS)
	$(CC) -c -o hlm_nobuf.o      bdbm_ftl/hlm_nobuf.c       $(CFLAGS)
	$(CC) -c -o host_block.o     bdbm_ftl/host_block.c      $(CFLAGS)
	$(CC) -c -o llm_noq.o        bdbm_ftl/llm_noq.c         $(CFLAGS)
	$(CC) -c -o dm_dragonfire.o  bdbm_ftl/dm_dragonfire.c   $(CFLAGS)
	$(CC) -c -o dev_dragonfire.o bdbm_ftl/dev_dragonfire.c  $(CFLAGS)

obj_ftl2:
	$(CC) -c -o abm.o      bdbm_ftl/ftl/abm.c      $(CFLAGS)
	$(CC) -c -o page_ftl.o bdbm_ftl/ftl/page_ftl.c $(CFLAGS)

obj_proc:
	$(CXX) -c -o add_buf10.o in_proc/add_buf10.cpp $(CFLAGS) -std=c++11
	$(CXX) -c -o tpch_query14.o in_proc/tpch_query14.cpp $(CFLAGS) -std=c++11
	$(CXX) -c -o tpch_query14_cmp_all.o in_proc/tpch_query14_cmp_all.cpp $(CFLAGS) -std=c++11
	$(CXX) -c -o tpch_query1.o in_proc/tpch_query1.cpp $(CFLAGS) -std=c++11
	$(CXX) -c -o tpch_query1_cmp_all.o in_proc/tpch_query1_cmp_all.cpp $(CFLAGS) -std=c++11
	$(CXX) -c -o afc_sticky.o in_proc/afc_sticky.cpp $(CFLAGS) -std=c++11
	$(CXX) -c -o join.o in_proc/join.cpp $(CFLAGS) -std=c++11

fsluissd: obj_nvme obj_ftl obj_ftl2 obj_proc
	$(CXX) -o issd_nvme $(OBJ) $(CFLAGS)

clean:
	rm -f *.o
	rm -f issd_nvme issd_nvme_stda

.PHONY: obj_nvme obj_ftl obj_ftl2 fsluissd clean all
