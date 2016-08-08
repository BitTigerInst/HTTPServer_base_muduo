include ../../common.mk

INCLUDE=-I../../../

BASE_BOJS=$(wildcard ../../base/*.o)
OBJS=$(wildcard ../*.o)
TARGET=$(patsubst %.cc,%,$(wildcard *.cc))

#Logging_test: Logging_test.o $(OBJS)
#	$(CC) $(CFLAGS) -o Logging_test Logging_test.o  $(OBJS)  $(LD)
all: $(TARGET)

$(TARGET): % : %.o
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $<  $(OBJS) $(BASE_BOJS)   $(LD)

%.o: %.cc
	$(CC) $(CFLAGS) $(INCLUDE) -c $< $(LD)

clean:
	rm -rf *.o
	rm $(TARGET)



	
