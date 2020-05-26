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
    char* message;
    char* dataLengthStr;
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

    dataLengthStr = (char*)calloc(sizeof(char), DIR_LEN+1);

    dataLengthStr = readFromFifoPipe(fd_client_r, arguments->bufferSize);

    dataLength = atoi(dataLengthStr);

    for (int i = 0; i < dataLength; i++){

        message = readFromFifoPipe(fd_client_r, arguments->bufferSize);

        newNodeItem = (struct DirListItem*)malloc(sizeof(struct DirListItem));
        newNodeItem->dirName = (char*)calloc(sizeof(char),DIR_LEN);
        newNodeItem->dirPath = (char*)calloc(sizeof(char),DIR_LEN);

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

        printf("Message Received: %s\n", newNodeItem->dirPath);
        free(message);
    }


    cmdManager = read_directory_list(cmdManager);

    /**
     * Send success message back to parent through clients fifo
     * */
    make_fifo_name_client_server(cmdManager->workerInfo->workerPid, cmdManager->workerInfo->workerFileName);
    createNewFifoPipe(cmdManager->workerInfo->workerFileName);
    fd_client_w = openFifoToWrite(cmdManager->workerInfo->workerFileName);

    message = (char*)calloc(sizeof(char),DIR_LEN+1);
    message = "Worker with pid has started...\n";
    writeInFifoPipe(fd_client_w, message, arguments->bufferSize);

    fflush(stdout);
    free(arguments);

    close(fd_client_w);
    close(fd_client_r);
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

    exit(0);
}