#macros 

CC = gcc
CFLAGS = -g
TARGET = oss
TARGET2 = child
LIBS = 
SRC = main.c detachAndRemove.c childPro.c setUpQueues.c
OBJOSS = main.o detachAndRemove.o setUpQueues.o
OBJCHILD = childPro.o

#explicit rule

all: $(TARGET) $(TARGET2)

$(TARGET): $(OBJOSS)
	$(CC) -pthread -o $(TARGET) $(OBJOSS)

$(TARGET2): $(OBJCHILD)
	$(CC) -pthread -o $(TARGET2) $(OBJCHILD)

#implicit rules 

main.o: main.c optArg.h detachAndRemove.h sharedMemory.h controlBlock.h setUpQueues.h
	$(CC) -c $(CFLAGS) main.c

childPro.o: childPro.c sharedMemory.h
	$(CC) -c $(CFLAGS) childPro.c

detachAndRemove.o: detachAndRemove.c detachAndRemove.h
	$(CC) -c $(CFLAGS) detachAndRemove.c

setUpQueues.o: setUpQueues.c setUpQueues.h
	$(CC) -c $(CFLAGS) setUpQueues.c



#cleaning up

clean:
	/bin/rm -f *.o $(TARGET) $(TARGET2)
