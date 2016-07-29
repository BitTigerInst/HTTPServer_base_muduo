#CC=g++
#CFLAGS=-std=c++11 -Wall -DMUDUO_STD_STRING
#LD=-lpthread -g 
include ../../common.mk

INCLUDE=-I../../../

BASE_BOJS=$(wildcard ../../base/*.o)
NET_OBJS=$(wildcard ../../net/*.o)
OBJS=$(wildcard ../*.o)
TARGET=$(patsubst %.cc,%,$(wildcard *.cc))

#Logging_test: Logging_test.o $(OBJS)
#	$(CC) $(CFLAGS) -o Logging_test Logging_test.o  $(OBJS)  $(LD)
all: $(TARGET)

$(TARGET): % : %.o
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $<  $(OBJS) $(NET_OBJS) $(BASE_BOJS)   $(LD)

%.o: %.cc
	$(CC) $(CFLAGS) $(INCLUDE) -c $< $(LD)

clean:
	rm -rf *.o
	rm $(TARGET)



	
