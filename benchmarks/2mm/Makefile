# makefile for 2mm from polybench

#CFLAGS := -DPOLYBENCH_TIME
CFLAGS := -DPOLYBENCH_GFLOPS
CFLAGS += -DPOLYBENCH_USE_C99_PROTO
#CFLAGS += -DMINI_DATASET
CFLAGS += -DEXTRALARGE_DATASET
CFLAGS += -DDATA_TYPE_IS_INT

2mm:
	$(CC) -o $@ $@.c $(CFLAGS) -I. polybench.c

clean:
	rm -f .o
	rm -f 2mm

.PHONY: 2mm clean
