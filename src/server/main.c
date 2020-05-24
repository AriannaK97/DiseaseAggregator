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


int main(int argc, char** argv){
    char *fifoName;
    Node* currentNode;
    DirListItem* item;
    int fd_client_w = -1;
    int status;
    char *listSize;
    time_t when;
    pid_t pid, endID;
/*****************************************************************************
 *                       Handling command line arguments                     *
 *****************************************************************************/

    AggregatorInputArguments* arguments = getAggregatorInputArgs(argc, argv);

/*****************************************************************************
 *                 Equal distribution of files between workers               *
 *****************************************************************************/

    AggregatorServerManager* aggregatorServerManager = readDirectoryFiles(arguments);
    //printAggregatorManagerDirectoryDistributor(aggregatorServerManager, arguments->numWorkers);

/*****************************************************************************
 *                               Start Server                                *
 *****************************************************************************/

    fprintf(stdout,"server has started...\n");

    aggregatorServerManager->numOfWorkers = arguments->numWorkers;
    aggregatorServerManager->workersArray = (struct WorkerInfo*)malloc(sizeof(struct WorkerInfo) * aggregatorServerManager->numOfWorkers);

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

        fifoName = malloc(sizeof(char) * DATA_SPACE);
        make_fifo_name(pid, fifoName, sizeof(fifoName));

        if(mkfifo(fifoName, PERM_FILE) == -1){
            if(errno != EEXIST) {
                perror("receiver: mkfifo");
                exit(6);
            }
        }

        if ( (fd_client_w=open(fifoName, O_WRONLY)) < 0){
            perror("fifo open error");
            exit(1);
        }

        listSize = (char*)malloc(sizeof(char) * DATA_SPACE);
        sprintf(listSize, "%d", aggregatorServerManager->directoryDistributor[i]->itemCount);
        if (write(fd_client_w, listSize, arguments->bufferSize) == -1){
            perror("Error in Writing");
            exit(2);
        }
        free(listSize);
        currentNode = (Node*)aggregatorServerManager->directoryDistributor[i]->head;
        while (currentNode != NULL){
            item = currentNode->item;
            if (write(fd_client_w, item->dirName, arguments->bufferSize) == -1){
                perror("Error in Writing");
                exit(2);
            }
            currentNode = currentNode->next;
        }

        close(fd_client_w);

        aggregatorServerManager->workersArray[i].serverFileName = malloc(sizeof(fifoName));
/*        strcpy(serverInfo->workersArray[i].serverFileName, fifoName);*/
        aggregatorServerManager->workersArray[i].workerPid = pid;

        for(i = 0; i < 15; i++) {
            endID = waitpid(pid, &status, WNOHANG|WUNTRACED);
            if (endID == -1) {            /* error calling waitpid       */
                perror("waitpid error");
                exit(EXIT_FAILURE);
            }
            else if (endID == 0) {        /* child still running         */
                time(&when);
                printf("Parent waiting for child at %s", ctime(&when));
                sleep(1);
            }
            else if (endID == pid) {  /* child ended                 */
                if (WIFEXITED(status))
                    printf("Child ended normally.n");
                else if (WIFSIGNALED(status))
                    printf("Child ended because of an uncaught signal.n");
                else if (WIFSTOPPED(status))
                    printf("Child process has stopped.n");
                exit(EXIT_SUCCESS);
            }
        }

    }


    freeAggregatorInputArguments(arguments);
    freeAggregatorManager(aggregatorServerManager);
    return 0;
}