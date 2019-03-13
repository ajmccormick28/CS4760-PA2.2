#macros 

CC = gcc
CFLAGS = -g
TARGET = oss
LIBS = 
SRC = main.c detachAndRemove.c
OBJFILEREAD = main.o detachAndRemove.o

#explicit rule

all: $(TARGET)

$(TARGET): $(OBJFILEREAD)
	$(CC) -o $(TARGET) $(OBJFILEREAD)

#implicit rules 

main.o: main.c optArg.h sharedTime.h detachAndRemove.h
	$(CC) -c $(CFLAGS) main.c

detachAndRemove.o: detachAndRemove.c detachAndRemove.h
	$(CC) -c $(CFLAGS) detachAndRemove.c

#cleaning up

clean:
	/bin/rm -f *.o $(TARGET)
