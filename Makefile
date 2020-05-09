OBJS	=	main.o	list_lib.o	data_io.o   hashTable.o redBlackTree.o  command_lib.o   binaryMaxHeap.o
SOURCE	=	src/main.c	src/list_lib.c	src/data_io.c   src/hashTable.c src/redBlackTree.c  src/command_lib.c   src/binaryMaxHeap.c
HEADER	=	header/list_lib.h	header/data_io.h   header/hashTable.h header/redBlackTree.h header/structs.h    header/command_lib.h    header/binaryMaxHeap.h
OUT	=   diseaseMonitor
CC	=	gcc
FLAGS   =	-Wall	-g	-c	-std=c99

$(OUT):	$(OBJS)
	$(CC)	-g	$(OBJS)	-o	$@

main.o: src/main.c
	$(CC)	$(FLAGS)	src/main.c

list_lib.o:   src/list_lib.c
	$(CC)	$(FLAGS)	src/list_lib.c

data_io.o:    src/data_io.c
	$(CC)	$(FLAGS)	src/data_io.c

hashTable.o:    src/hashTable.c
	$(CC)	$(FLAGS)	src/hashTable.c

redBlackTree.o:    src/redBlackTree.c
	$(CC)	$(FLAGS)	src/redBlackTree.c

command_lib.o:    src/command_lib.c
	$(CC)	$(FLAGS)	src/command_lib.c

binaryMaxHeap.o:    src/binaryMaxHeap.c
	$(CC)	$(FLAGS)	src/binaryMaxHeap.c

clean:
	rm	-f	$(OBJS)	$(OUT)

count:
	wc	$(SOURCE)	$(HEADER)