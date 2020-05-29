//
// Created by AriannaK97 on 26/5/20.
//

#include <fcntl.h>
#include "../../header/communication.h"


void createNewFifoPipe(char *fileName){
    if(mkfifo(fileName, PERM_FILE) == -1 && errno != EEXIST){
        perror("receiver: mkfifo");
        exit(6);
    }
}

int openFifoToRead(char *fileName){
    int fd_client_r = -1;
    if ( (fd_client_r=open(fileName, O_RDONLY)) < 0){
        perror("fifo open problem");
        exit(3);
    }
    printf("trying to open ffs %d\n", fd_client_r);
    return fd_client_r;
}

int openFifoToWrite(char *fileName){
    int fd_client_w = -1;
    if ( (fd_client_w=open(fileName, O_WRONLY)) < 0){
        perror("fifo open error");
        exit(1);
    }
    return fd_client_w;
}

void readFromFifoPipe(int fd_client_r, void* message, size_t bufferSize){
    if (read(fd_client_r, message, bufferSize) < 0) {
        perror("problem in reading");
        exit(5);
    }
    printf("read from pipe %d\n", fd_client_r);
}

void writeInFifoPipe(int fd_client_w, void* message , size_t bufferSize){
    if (write(fd_client_w, message, bufferSize) == -1){
        perror("Error in Writing");
        exit(2);
    }
}