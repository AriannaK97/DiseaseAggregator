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

char* readFromFifoPipe(int fd_client_r, size_t bufferSize){
    char *message = (char*)calloc(sizeof(char),DIR_LEN+1);
    if (read(fd_client_r, message, bufferSize) < 0) {
        perror("problem in reading");
        exit(5);
    }
    return message;
}

void writeInFifoPipe(int fd_client_w, char* message ,size_t bufferSize){
    if (write(fd_client_w, message, bufferSize) == -1){
        perror("Error in Writing");
        exit(2);
    }
}