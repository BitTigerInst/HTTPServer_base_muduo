CC=g++
CFLAGS=-I../../ -std=c++11 -Wall -DMUDUO_STD_STRING
LD=-lpthread -g
OBJS=$(patsubst %.cc,%.o,$(wildcard *.cc))

all: $(OBJS)

%.o: %.cc 
	$(CC) $(CFLAGS) $(LD) -c $<

clean:
	rm -rf *.o
