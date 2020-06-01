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
    Node* currentNode;
    DirListItem* item;
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
            char* bufferSize_str = (char*)malloc(sizeof(char)*(arguments->bufferSize)+1);
            sprintf(bufferSize_str, "%zu", arguments->bufferSize);
            execlp("./diseaseMonitor_client", "./diseaseMonitor_client", bufferSize_str, "5", "5" , "256",
                    arguments->input_dir,(char*)NULL);
            printf("Return not expected. Must be an execv error.n\n");
            free(bufferSize_str);
        }

        aggregatorServerManager->workersArray[i].workerPid = pid;
        aggregatorServerManager->workersArray[i].serverFileName = (char*)malloc(sizeof(char)*DIR_LEN);
        aggregatorServerManager->workersArray[i].workerFileName = (char*)malloc(sizeof(char)*DIR_LEN);


        /*make fifo pipe for server*/
        make_fifo_name_server_client(pid, aggregatorServerManager->workersArray[i].serverFileName);
        createNewFifoPipe(aggregatorServerManager->workersArray[i].serverFileName);
        aggregatorServerManager->workersArray[i].fd_client_w = openFifoToWrite(aggregatorServerManager->workersArray[i].serverFileName);

        /*send the length of the data the client has to read*/
        message = calloc(sizeof(char), aggregatorServerManager->bufferSize + 1);
        sprintf(message, "%d", aggregatorServerManager->directoryDistributor[i]->itemCount);
        writeInFifoPipe(aggregatorServerManager->workersArray[i].fd_client_w, message, (aggregatorServerManager->bufferSize) + 1);
        free(message);

        currentNode = (Node*)aggregatorServerManager->directoryDistributor[i]->head;
        while (currentNode != NULL){
            item = currentNode->item;
            /*write the size of the name of the directory to follow to fifo*/
            //printf("from server %d\n", messageSize);
            /*write the directory name to fifo*/
            message = calloc(sizeof(char), (aggregatorServerManager->bufferSize)+1);
            strcpy(message, item->dirName);
            writeInFifoPipe(aggregatorServerManager->workersArray[i].fd_client_w, message, (size_t)(aggregatorServerManager->bufferSize)+1);
            free(message);

            currentNode = currentNode->next;
        }


        /*start receiving*/
        make_fifo_name_client_server(pid, aggregatorServerManager->workersArray[i].workerFileName);
        createNewFifoPipe(aggregatorServerManager->workersArray[i].workerFileName);
        aggregatorServerManager->workersArray[i].fd_client_r = openFifoToRead(aggregatorServerManager->workersArray[i].workerFileName);

        /*read actual message from fifo*/
        message = calloc(sizeof(char), (arguments->bufferSize)+1);
        readFromFifoPipe(aggregatorServerManager->workersArray[i].fd_client_r, message,(arguments->bufferSize)+1);

        fprintf(stdout, "%s\n", message);

        if(!receiveStats(aggregatorServerManager, i)){
            fprintf(stderr, "Could not receive statistics\n");
        }

        free(message);
    }

    DiseaseAggregatorServerManager(aggregatorServerManager);

    for (int j = 0; j < aggregatorServerManager->numOfWorkers; ++j) {
        close(aggregatorServerManager->workersArray[j].fd_client_w);
        close(aggregatorServerManager->workersArray[j].fd_client_r);
    }


    freeAggregatorInputArguments(arguments);
    freeAggregatorManager(aggregatorServerManager);
    return 0;
}