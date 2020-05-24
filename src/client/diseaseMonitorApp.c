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

int main(int argc, char** argv) {
    WorkerInfo workerInfo;
    char *fifoName;
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
/*    fprintf(stdout,"%ld, %ld, %d, %d, %s\n", arguments->bufferSize, arguments->bucketSize,
            arguments->countryHashTableNumOfEntries, arguments->diseaseHashtableNumOfEntries, arguments->input_dir);*/
    fprintf(stdout, "I am the client\n");
/*****************************************************************************
 *                       Handling input files                                *
 *****************************************************************************/


    cmdManager = initializeStructures(arguments);
    workerInfo.workerPid = getpid();
    fifoName = malloc(sizeof(char) * DATA_SPACE);
    make_fifo_name(workerInfo.workerPid, fifoName, sizeof(fifoName));

    if(mkfifo(fifoName, PERM_FILE) == -1 && errno != EEXIST){
        perror("receiver: mkfifo");
        exit(6);
    }

    if ( (fd_client_r=open(fifoName, O_RDWR)) < 0){
        perror("fifo open problem");
        exit(3);
    }

    dataLengthStr = (char*)calloc(sizeof(char), DIR_LEN+1);
    if (read(fd_client_r, dataLengthStr, arguments->bufferSize) < 0) {
        perror("problem in reading");
        exit(5);
    }
    dataLength = atoi(dataLengthStr);

    for (int i = 0; i < dataLength; i++){

        message = (char*)calloc(sizeof(char),DIR_LEN+1);
        if (read(fd_client_r, message, arguments->bufferSize) < 0) {
            perror("problem in reading");
            exit(5);
        }

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

       // printf("Message Received: %s\n", newNodeItem->dirPath);
        free(message);
    }


    cmdManager = read_directory_list(cmdManager);
    //printList(cmdManager->patientList);
    fflush(stdout);
    free(arguments);
    //close(fd_client_r);
    //fclose(patientRecordsFile);

    /**
     * Uncomment the line below to see all the inserted patients in the list
     * */




    /**
     * Uncomment the two lines below to see the hashtable contents
     * */
    //applyOperationOnHashTable(cmdManager->diseaseHashTable, PRINT);
    //applyOperationOnHashTable(cmdManager->countryHashTable, PRINT);

    //commandServer(cmdManager);
    fprintf(stdout, "exiting child\n");
    close(fd_client_r);
    exit(0);
}