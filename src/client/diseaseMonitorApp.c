//
// Created by AriannaK97 on 22/5/20.
//

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include "../../header/data_io.h"
#include "../../header/command_lib.h"
#include "../../header/diseaseAggregator.h"
#include "../../header/communication.h"

int main(int argc, char** argv) {

    int fd_client_r = -1;
    int fd_client_w = -1;
    int messageSize;
    char* message;
    int dataLengthStr;
    int dataLength;
    DirListItem* newNodeItem = NULL;
    Node* newNode = NULL;
    CmdManager* cmdManager;

/*****************************************************************************
 *                       Handling command line arguments                     *
 *****************************************************************************/
    MonitorInputArguments* arguments = getMonitorInputArgs(argc, argv);
/*****************************************************************************
 *                       Handling input files                                *
 *****************************************************************************/


    cmdManager = initializeStructures(arguments);
    cmdManager->workerInfo->workerPid = getpid();

    make_fifo_name_server_client(cmdManager->workerInfo->workerPid, cmdManager->workerInfo->serverFileName);
    createNewFifoPipe(cmdManager->workerInfo->serverFileName);

    fd_client_r = openFifoToRead(cmdManager->workerInfo->serverFileName);

    /*receive from server the length of data the client will receive*/
    readFromFifoPipe(fd_client_r, &dataLength, sizeof(int));

    for (int i = 0; i < dataLength; i++){

        /*receive the size of the incoming message from fifo*/
        readFromFifoPipe(fd_client_r, &messageSize, sizeof(int));

        /*read actual message from fifo*/
        if (messageSize > arguments->bufferSize) {
            message = malloc(sizeof(char) * messageSize + 1);
            readFromFifoPipe(fd_client_r, message, messageSize);
        }else{
            message = malloc(sizeof(char)*(arguments->bufferSize)+1);
            readFromFifoPipe(fd_client_r, message,arguments->bufferSize);
            messageSize = arguments->bufferSize;
        }

        newNodeItem = (struct DirListItem*)malloc(sizeof(struct DirListItem));
        newNodeItem->dirName = (char*)malloc(sizeof(char)*DIR_LEN);
        newNodeItem->dirPath = (char*)malloc(sizeof(char)*DIR_LEN);

        strcpy(newNodeItem->dirName, message);
        strcpy(newNodeItem->dirPath, arguments->input_dir);
        strcat(newNodeItem->dirPath, "/");
        strcat(newNodeItem->dirPath, message);

        newNode = nodeInit(newNodeItem);
        if(cmdManager->directoryList == NULL){
            cmdManager->directoryList = linkedListInit(newNode);
        } else{
            push(newNode, cmdManager->directoryList);
        }

        printf("Message Received: %s\n", newNodeItem->dirName);

        free(message);
    }

    /*receive end of transmission from server*/
    int noMessage = -1;
    while (noMessage != 0){
        readFromFifoPipe(fd_client_r, &noMessage, sizeof(int));
    }

    //cmdManager = read_directory_list(cmdManager);

    /**
     * Send success message back to parent through clients fifo
     * */
    make_fifo_name_client_server(cmdManager->workerInfo->workerPid, cmdManager->workerInfo->workerFileName);
    createNewFifoPipe(cmdManager->workerInfo->workerFileName);
    fd_client_w = openFifoToWrite(cmdManager->workerInfo->workerFileName);

    message = (char*)calloc(sizeof(char),DIR_LEN);
    strcpy(message, "Worker with pid has started...\n");
    messageSize = strlen(message);
    /*write the size of the name of the directory to follow to fifo*/
    writeInFifoPipe(fd_client_w, &messageSize, sizeof(int));
    /*write the directory name to fifo*/
    if(messageSize > arguments->bufferSize)
        writeInFifoPipe(fd_client_w, message, (size_t)messageSize);
    else
        writeInFifoPipe(fd_client_w, message, arguments->bufferSize);

    fflush(stdout);
    free(arguments);


    /**
     * Uncomment the line below to see all the inserted patients in the list
     * */

    //printList(cmdManager->patientList);


    /**
     * Uncomment the two lines below to see the hashtable contents
     * */
    //applyOperationOnHashTable(cmdManager->diseaseHashTable, PRINT);
    //applyOperationOnHashTable(cmdManager->countryHashTable, PRINT);

    //commandServer(cmdManager);

    fprintf(stdout, "exiting child\n");
    close(fd_client_w);
    close(fd_client_r);
    exit(0);
}