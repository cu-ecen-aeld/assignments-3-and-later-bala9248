CC= $(CROSS_COMPILE)gcc
CFLAGS= -g -Wall 
TARGET= aesdsocket
SRCS:= aesdsocket.c
LDFLAGS= -pthread 
default: $(TARGET)
all: $(TARGET)


$(TARGET): $(TARGET).o
	$(CC) $(CFLAGS) $(LDFLAGS) $(TARGET).o -o $(TARGET)
	
$(TARGET).o: $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o $(TARGET).o $(SRCS)

clean: 	
	rm -f *.o $(TARGET) *.elf *.map
