#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include "../../header/diseaseAggregator.h"
#include "../../header/server.h"

int main(int argc, char** argv){
    int aggregatorServer, worker;
    ssize_t nread;
    Message msg;
    char *fifoName;
    Node* currentNode;
    DirListItem* item;
    CmdManager* cmdManager;
    FILE *fd;
    int status;
    char myinputparam[20];
    pid_t pid;
/*****************************************************************************
 *                       Handling command line arguments                     *
 *****************************************************************************/

    AggregatorInputArguments* arguments = getAggregatorInputArgs(argc, argv);

/*****************************************************************************
 *                 Equal distribution of files between workers               *
 *****************************************************************************/

    AggregatorManager* aggregatorManager = readDirectoryFiles(arguments);
    printAggregatorManagerDirectoryDistributor(aggregatorManager, arguments->numWorkers);

/*****************************************************************************
 *                               Start Server                                *
 *****************************************************************************/

    fprintf(stdout,"server has started...\n");

    ServerInfo *serverInfo = malloc(sizeof(ServerInfo));
    serverInfo->numOfWorkers = arguments->numWorkers;
    serverInfo->workersArray = malloc(sizeof(WorkerInfo) * serverInfo->numOfWorkers);

    /*create named pipe for the aggregator/server*/
    if(mkfifo(SERVER_FIFO_NAME, PERM_FILE) == -1 && errno != EEXIST){
        perror("receiver: mkfifo");
        exit(6);
    }

    /*create workers*/
    for(int i = 0; i < arguments->numWorkers; i++){
        fifoName = malloc(sizeof(char) * 100);
        make_fifo_name(i, fifoName, sizeof(fifoName));

        if(mkfifo(fifoName, PERM_FILE) == -1 && errno != EEXIST){
            perror("receiver: mkfifo");
            exit(6);
        }

        if ((pid = fork()) == -1) {
            perror("fork error");
            exit(1);
        }else if (pid == 0) {
            char* bufferSize_str = malloc(sizeof(char)*DATA_SPACE);
            sprintf(bufferSize_str, "%zu", arguments->bufferSize);
            char *const paramList[] = {"diseaseMonitorApp.c", bufferSize_str, "5", "5" , "256", NULL};
            execv("/bin/ls", paramList);
            printf("Return not expected. Must be an execv error.n");
        }

        serverInfo->workersArray[i].serverFileName = malloc(sizeof(fifoName));
        strcpy(serverInfo->workersArray[i].serverFileName, fifoName);
        serverInfo->workersArray[i].workerPid = pid;

    }


    if ( pid!=0 ){          // parent process - closes off everything
        if ( (fd=open(SERVER_FIFO_NAME, O_WRONLY| O_NONBLOCK)) < 0){
            perror("fife open error"); exit(1);
        }
        if (wait(&status)!=pid){
            perror("Waiting for child\n"); exit(1);
        }
        else 	printf("Just synched with child\n");
    }
    else {
        fprintf("%s\n", fifoName);
        char* bufferSize_str = malloc(sizeof(char)*DATA_SPACE);
        sprintf(bufferSize_str, "%zu", arguments->bufferSize);
        char *const parmList[] = {"diseaseMonitor_client", bufferSize_str, "5", "5" , "256", "NULL"};
        execv("/home/linuxuser/CLionProjects/DiseaseAggregator/diseaseMonitor_client", parmList);
        perror("execlp");
    }

/*    for (int i = 0; i < arguments->numWorkers; ++i) {
        currentNode = (Node*)aggregatorManager->directoryDistributor[i]->head;
        while (currentNode != NULL){
            cmdManager = read_directory_list(aggregatorManager->directoryDistributor[i]);
            currentNode = currentNode->next;
        }
        break;
    }*/

    return 0;
}