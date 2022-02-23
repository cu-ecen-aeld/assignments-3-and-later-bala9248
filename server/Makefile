ifeq ($(CC),)
	CC= $(CROSS_COMPILE)gcc
endif

ifeq ($(CFLAGS),)
	CFLAGS= -g -Wall
endif
 
ifeq ($(LDFLAGS),)
	LDFLAGS= -pthread 
endif

TARGET= aesdsocket
SRCS:= aesdsocket.c

default: $(TARGET)
all: $(TARGET)


$(TARGET): $(SRCS) 
	$(CC) $(CFLAGS) $^ -o $@ $(INCLUDES) $(LDFLAGS)


clean: 	
	rm -f *.o $(TARGET) *.elf *.map
