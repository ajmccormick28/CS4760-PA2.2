#macros 

CC = gcc
CFLAGS = -g
TARGET = master
TARGET2 = palin
LIBS = 
SRC = main.c palin.c detachAndRemove.c getNamed.c palinCheck.c
OBJFILEREAD = main.o detachAndRemove.o getNamed.o
OBJCHILD = palin.o getNamed.o palinCheck.o

#explicit rule

all: $(TARGET) $(TARGET2)

$(TARGET): $(OBJFILEREAD)
	$(CC) -pthread -o $(TARGET) $(OBJFILEREAD)

$(TARGET2): $(OBJCHILD)
	$(CC) -pthread -o $(TARGET2) $(OBJCHILD)

#implicit rules 

main.o: main.c optArg.h inputHold.h detachAndRemove.h getNamed.h
	$(CC) -c $(CFLAGS) main.c

user.o: palin.c inputHold.h getNamed.h palinCheck.h
	$(CC) -c $(CFLAGS) user.c

detachAndRemove.o: detachAndRemove.c detachAndRemove.h
	$(CC) -c $(CFLAGS) detachAndRemove.c

getNamed.o: getNamed.c getNamed.h
	$(CC) -c $(CFLAGS) getNamed.c

palinCheck.o: palinCheck.c palinCheck.h
	$(CC) -c $(CFLAGS) palinCheck.c

#cleaning up

clean:
	/bin/rm -f *.o $(TARGET) $(TARGET2)
