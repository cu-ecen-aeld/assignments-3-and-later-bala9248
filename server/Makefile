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


$(TARGET): $(TARGET).o
	$(CC) $(CFLAGS) $(LDFLAGS) $(TARGET).o -o $(TARGET)
	
$(TARGET).o: $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o $(TARGET).o $(SRCS)

clean: 	
	rm -f *.o $(TARGET) *.elf *.map
