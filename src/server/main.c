#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include "../../header/diseaseAggregator.h"
#include "../../header/communication.h"


int main(int argc, char** argv){
    char *fifoName;
    Node* currentNode;
    DirListItem* item;
    int fd_client_w = -1;
    int fd_client_r = -1;
    int messageSize;
    char *listSize;
    char *message;
    pid_t pid;
/*****************************************************************************
 *                       Handling command line arguments                     *
 *****************************************************************************/

    AggregatorInputArguments* arguments = getAggregatorInputArgs(argc, argv);

/*****************************************************************************
 *                 Equal distribution of files between workers               *
 *****************************************************************************/

    AggregatorServerManager* aggregatorServerManager = readDirectoryFiles(arguments);
    //printAggregatorManagerDirectoryDistributor(DiseaseAggregatorServerManager, arguments->numWorkers);

/*****************************************************************************
 *                               Start Server                                *
 *****************************************************************************/

    fprintf(stdout,"server has started...\n");

    aggregatorServerManager->bufferSize = arguments->bufferSize;
    aggregatorServerManager->numOfWorkers = arguments->numWorkers;
    aggregatorServerManager->workersArray = (WorkerInfo*)malloc(sizeof(WorkerInfo) * aggregatorServerManager->numOfWorkers);

    /*create workers*/
    for(int i = 0; i < arguments->numWorkers; i++){

        if ((pid = fork()) == -1) {
            perror("fork error");
            exit(1);
        }else if (pid == 0) {
            char* bufferSize_str = (char*)malloc(arguments->bufferSize);
            sprintf(bufferSize_str, "%zu", arguments->bufferSize);
            execlp("./diseaseMonitor_client", "./diseaseMonitor_client", bufferSize_str, "5", "5" , "256",
                    arguments->input_dir,(char*)NULL);
            printf("Return not expected. Must be an execv error.n\n");
            free(bufferSize_str);
        }

        aggregatorServerManager->workersArray[i].workerPid = pid;
        aggregatorServerManager->workersArray[i].serverFileName = (char*)calloc(sizeof(char), DIR_LEN);
        aggregatorServerManager->workersArray[i].workerFileName = (char*)calloc(sizeof(char),DIR_LEN);


        /*make fifo pipe for server*/
        make_fifo_name_server_client(pid, aggregatorServerManager->workersArray[i].serverFileName);
        createNewFifoPipe(aggregatorServerManager->workersArray[i].serverFileName);
        fd_client_w = openFifoToWrite(aggregatorServerManager->workersArray[i].serverFileName);

        /*send the length of the data the client has to read*/
        writeInFifoPipe(fd_client_w, &(aggregatorServerManager->directoryDistributor[i]->itemCount), sizeof(int));

        currentNode = (Node*)aggregatorServerManager->directoryDistributor[i]->head;
        while (currentNode != NULL){
            item = currentNode->item;
            messageSize = strlen(item->dirName);
            /*write the size of the name of the directory to follow to fifo*/
            //printf("%d\n", messageSize);
            writeInFifoPipe(fd_client_w, &messageSize, sizeof(int));
            /*write the directory name to fifo*/
            if(messageSize > arguments->bufferSize)
                writeInFifoPipe(fd_client_w, item->dirName, (size_t)messageSize);
            else
                writeInFifoPipe(fd_client_w, item->dirName, arguments->bufferSize);
            currentNode = currentNode->next;
        }

        /*transmit end of sending data*/
        int noMessage = 0;
        writeInFifoPipe(fd_client_w, &noMessage, sizeof(int));

        close(fd_client_w);

        /*start receiving*/
        make_fifo_name_client_server(pid, aggregatorServerManager->workersArray[i].workerFileName);
        createNewFifoPipe(aggregatorServerManager->workersArray[i].workerFileName);
        fd_client_r = openFifoToRead(aggregatorServerManager->workersArray[i].workerFileName);
        /*receive the size of the incoming message from fifo*/
        readFromFifoPipe(fd_client_r, &messageSize, sizeof(int));
        /*read actual message from fifo*/
        message = calloc(sizeof(char), messageSize+1);
        if(messageSize > arguments->bufferSize)
            readFromFifoPipe(fd_client_r, message,messageSize);
        else
            readFromFifoPipe(fd_client_r, message,arguments->bufferSize);

        fprintf(stdout, "%s\n", message);
        free(message);
        close(fd_client_r);

    }

    for (int j = 0; j < aggregatorServerManager->numOfWorkers; ++j) {
        wait(NULL);
    }

    DiseaseAggregatorServerManager(aggregatorServerManager);


    freeAggregatorInputArguments(arguments);
    freeAggregatorManager(aggregatorServerManager);
    return 0;
}