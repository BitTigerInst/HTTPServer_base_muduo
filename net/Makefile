include ../common.mk

INCLUDE=-I../../
OBJS=$(patsubst %.cc,%.o,$(wildcard *.cc))

all: $(OBJS)

%.o: %.cc 
	$(CC) $(CFLAGS) $(INCLUDE) $(LD) -c $<

clean:
	rm -rf *.o
