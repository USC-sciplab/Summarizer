# Makefile for tpch_query14
#SRC = $(wildcard *.cpp)
SRC = nvme_tpch_query14.cpp \
	  tpch_query14_baseline.cpp \
	  tpch_query14_ssd_all.cpp \
	  tpch_query14_ssd_mix.cpp \
	  tpch_query14_ssd_cmp.cpp 
OBJ = $(patsubst %.cpp, %.o, $(SRC))

CFLAGS := -m64 -lm -lrt -std=c++11
CFLAGS += -I. -I../util

# SSD processing mode
# 0: baseline
# 1: all processing at ssd
# 2: comparision processing at ssd
# 3: mixing baseline + all processing at ssd
CFLAGS += -DSSD_PROC_MODE=3
# baseline function delay
#CFLAGS += -DGK_FUNC_DELAY=0
#CFLAGS += -DGK_FUNC_DELAY=42
#CFLAGS += -DGK_FUNC_DELAY=2814

all: query

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CFLAGS)

util:
	$(CXX) -c -o util.o ../util/util.c $(CFLAGS)

wr_db: clean util
	$(CXX) -c -o nvme_tpch_query14_wr_db.o nvme_tpch_query14_wr_db.cpp $(CFLAGS)
	$(CXX) -o nvme_tpch_query14_wr_db nvme_tpch_query14_wr_db.o util.o $(CFLAGS)

query: clean $(OBJ) util
	$(CXX) -o nvme_tpch_query14 $(OBJ) util.o $(CFLAGS)

clean:
	rm -f *.o

purge: clean
	rm -f nvme_tpch_query14
	rm -f nvme_tpch_query14_wr_db

.PHONY: clean purge util wr_db query
