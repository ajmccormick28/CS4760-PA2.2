#macros 

CC = gcc
CFLAGS = -g
TARGET = oss
TARGET2 = child
LIBS = 
SRC = main.c detachAndRemove.c childPro.c
OBJOSS = main.o detachAndRemove.o
OBJCHILD = childPro.o

#explicit rule

all: $(TARGET) $(TARGET2)

$(TARGET): $(OBJOSS)
	$(CC) -pthread -o $(TARGET) $(OBJOSS)

$(TARGET2): $(OBJCHILD)
	$(CC) -pthread -o $(TARGET2) $(OBJCHILD)

#implicit rules 

main.o: main.c optArg.h detachAndRemove.h sharedTime.h controlBlock.h
	$(CC) -c $(CFLAGS) main.c

childPro.o: childPro.c sharedTime.h
	$(CC) -c $(CFLAGS) childPro.c

detachAndRemove.o: detachAndRemove.c detachAndRemove.h
	$(CC) -c $(CFLAGS) detachAndRemove.c





#cleaning up

clean:
	/bin/rm -f *.o $(TARGET) $(TARGET2)
