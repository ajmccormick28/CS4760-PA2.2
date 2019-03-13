#macros 

CC = gcc
CFLAGS = -g
TARGET = oss
TARGET2 = user
LIBS = 
SRC = main.c user.c detachAndRemove.c
OBJFILEREAD = main.o detachAndRemove.o
OBJCHILD = user.o

#explicit rule

all: $(TARGET) $(TARGET2)

$(TARGET): $(OBJFILEREAD)
	$(CC) -o $(TARGET) $(OBJFILEREAD)

$(TARGET2): $(OBJCHILD)
	$(CC) -o $(TARGET2) $(OBJCHILD)

#implicit rules 

main.o: main.c optArg.h sharedTime.h detachAndRemove.h
	$(CC) -c $(CFLAGS) main.c

user.o: user.c sharedTime.h
	$(CC) -c $(CFLAGS) user.c

detachAndRemove.o: detachAndRemove.c detachAndRemove.h
	$(CC) -c $(CFLAGS) detachAndRemove.c

#cleaning up

clean:
	/bin/rm -f *.o $(TARGET)
