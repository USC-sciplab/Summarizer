# makefile
SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, %.o, $(SRC))

CFLAGS := -g
CFLAGS += -lrt

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

time_usleep: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	rm -f *.o
	rm -f time_usleep
