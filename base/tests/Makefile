CC=g++
CFLAGS=-I../../../ -std=c++11 -Wall
LD=-lpthread -O2 -lboost_unit_test_framework

OBJS=$(wildcard ../*.o)
TARGET=$(patsubst %.cc,%,$(wildcard *.cc))

#Logging_test: Logging_test.o $(OBJS)
#	$(CC) $(CFLAGS) -o Logging_test Logging_test.o  $(OBJS)  $(LD)
all: $(TARGET)

$(TARGET): % : %.o
	$(CC) $(CFLAGS) -o $@ $<  $(OBJS)  $(LD)

%.o: %.cc
	$(CC) $(CFLAGS) -c $< $(LD)

clean:
	rm -rf *.o
	rm $(TARGET)



	
