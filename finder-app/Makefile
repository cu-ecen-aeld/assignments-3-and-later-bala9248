CC= $(CROSS_COMPILE)gcc
CFLAGS= -g -Wall 
TARGET= writer
SRCS:= writer.c


$(TARGET): $(TARGET).o
	$(CC) $(CFLAGS)  $(TARGET).o -o $(TARGET)
	
$(TARGET).o: $(SRCS)
	$(CC) $(CFLAGS) -c -o $(TARGET).o $(SRCS)

clean: 	
	rm -f *.o $(TARGET) *.elf *.map
