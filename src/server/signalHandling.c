//
// Created by AriannaK97 on 2/6/20.
//

#include "../../header/signalHandling.h"
#include "../../header/structs.h"

void sigintHandler(int sig){
    FILE * fptr;
    DirListItem* item;
    Node* node = globDirList->head;
    char* fileName = calloc(sizeof(char), 15);
    sprintf(fileName, "log_file.%d", globPid);
    fptr = fopen(fileName, "w");
    if(fptr < 0){
        /* File not created hence exit */
        perror("could not create log file for worker- ");
        exit(3);
    }

    while (node != NULL) {
        item = (DirListItem*)node->item;
        fprintf(fptr, "%s\n",item->dirName);
        node = node->next;
    }

    fprintf(fptr, "TOTAL %d\nSUCCESS %d\nFAIL %d", globWorkerLog->successes+globWorkerLog->fails,
            globWorkerLog->successes, globWorkerLog->fails);
    fprintf(stdout, "Please help - My dad is killing me...\n DEAD\n");

    fclose(fptr);
    free(fileName);
}