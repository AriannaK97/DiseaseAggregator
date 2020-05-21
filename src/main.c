#include <stdio.h>
#include <sys/stat.h>
#include "../header/data_io.h"
#include "../header/command_lib.h"
#include "../header/diseaseAggregator.h"

int main(int argc, char** argv){
    int aggregatorServer, worker, i;
    ssize_t nread;
    Message msg;
    char fifoName[100];
    Node* currentNode;
    DirListItem* item;
    CmdManager* cmdManager;
/*****************************************************************************
 *                       Handling command line arguments                     *
 *****************************************************************************/

    InputArguments* arguments = getInputArgs(argc, argv);
    AggregatorManager* aggregatorManager = readDirectoryFiles(arguments);
    printAggregatorManagerDirectoryDistributor(aggregatorManager, arguments->numWorkers);
    fprintf(stdout,"Server has started...\n");

    for (int i = 0; i < arguments->numWorkers; ++i) {
        currentNode = (Node*)aggregatorManager->directoryDistributor[i]->head;
        while (currentNode != NULL){
            cmdManager = read_directory_list(aggregatorManager->directoryDistributor[i]);
            currentNode = currentNode->next;
        }
        break;
    }

    //if(mkfifo(SERVER_FIFO_NAME, PERM_FILE))
    return 0;
}