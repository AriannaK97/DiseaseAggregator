CC	=	gcc
FLAGS   =	-Wall	-g	-c	-std=c99

all: diseaseAggregator_server diseaseMonitor_client

diseaseAggregator_server:   main.o  diseaseAggregator.o list_lib.o	 data_io.o   redBlackTree.o  command_lib.o   hashTable.o	binaryMaxHeap.o
	$(CC)   -o diseaseAggregator_server main.o diseaseAggregator.o list_lib.o	 data_io.o   redBlackTree.o  command_lib.o   hashTable.o	binaryMaxHeap.o

diseaseMonitor_client:  diseaseMonitorApp.o data_io.o   redBlackTree.o  command_lib.o   hashTable.o list_lib.o  binaryMaxHeap.o	diseaseAggregator.o
	$(CC)   -o	diseaseMonitor_client	diseaseMonitorApp.o data_io.o   redBlackTree.o  command_lib.o   hashTable.o list_lib.o  binaryMaxHeap.o	diseaseAggregator.o

main.o: src/server/main.c
	$(CC)	$(FLAGS)	src/server/main.c

list_lib.o:   src/client/list_lib.c
	$(CC)	$(FLAGS)	src/client/list_lib.c

data_io.o:    src/client/data_io.c
	$(CC)	$(FLAGS)	src/client/data_io.c

hashTable.o:    src/client/hashTable.c
	$(CC)	$(FLAGS)	src/client/hashTable.c

redBlackTree.o:    src/client/redBlackTree.c
	$(CC)	$(FLAGS)	src/client/redBlackTree.c

command_lib.o:    src/client/command_lib.c
	$(CC)	$(FLAGS)	src/client/command_lib.c

binaryMaxHeap.o:    src/client/binaryMaxHeap.c
	$(CC)	$(FLAGS)	src/client/binaryMaxHeap.c

diseaseAggregator.o:    src/server/diseaseAggregator.c
	$(CC)	$(FLAGS)	src/server/diseaseAggregator.c

server.o:    src/server/server.c
	$(CC)	$(FLAGS)	src/server/server.c

diseaseMonitorApp.o: src/client/diseaseMonitorApp.c
	$(CC)	$(FLAGS)	src/client/diseaseMonitorApp.c

clean:
	rm	-rf *.o diseaseMonitor_client diseaseAggregator_server
	rm -rf worker*
	rm -rf aggregator_server

count:
	wc	$(SOURCE)	$(HEADER)