//
// Created by linuxuser on 21/5/20.
//
#define  _GNU_SOURCE
#include <sys/wait.h>
#include <stdio.h>
#include "../../header/diseaseAggregator.h"
#include "../../header/command_lib.h"
#include "../../header/communication.h"


AggregatorServerManager* readDirectoryFiles(AggregatorInputArguments* arguments){
    DIR* FD;
    DIR* SubFD;
    struct dirent* in_dir;
    char *dirPath = (char*)malloc(DIR_LEN*sizeof(char));
    char *subDirPath = (char*)malloc(DIR_LEN*sizeof(char));
    AggregatorServerManager* aggregatorManager;
    int distributionPointer = 0;
    DirListItem* newItem = NULL;
    Node* listNode = NULL;

    /* Scanning the in directory */
    if (NULL == (FD = opendir (arguments->input_dir))){
        fprintf(stderr, "Error : Failed to open input directory - %s\n", strerror(errno));
        exit(1);
    }
    strcpy(dirPath, arguments->input_dir);
    strcat(dirPath, "/");
    int numOfSubDirectories = 0;
    while((in_dir = readdir(FD))){
        numOfSubDirectories+=1;
    }
    rewinddir(FD);

    aggregatorManager = (AggregatorServerManager*)malloc(sizeof(AggregatorServerManager));
    /*array of lists - each list holds the directories assigned to each worker*/
    aggregatorManager->directoryDistributor = (List**)calloc(sizeof(List*),arguments->numWorkers);
    for (int i = 0; i < arguments->numWorkers; ++i) {
        aggregatorManager->directoryDistributor[i] = NULL;
    }

    while ((in_dir = readdir(FD))){
        if(distributionPointer >= arguments->numWorkers){
            distributionPointer = 0;
        }
        if (!strcmp (in_dir->d_name, "."))
            continue;
        if (!strcmp (in_dir->d_name, ".."))
            continue;
        /*create current deirectory's path*/
        strcpy(subDirPath, dirPath);
        strcat(subDirPath, in_dir->d_name);

        /* Open subdirestory*/
        if (NULL == (SubFD = opendir (subDirPath))){
            fprintf(stderr, "Error : Failed to open input directory %s - %s\n",subDirPath, strerror(errno));
            exit(1);
        }

        newItem = (struct DirListItem*)malloc(sizeof(struct DirListItem));
        newItem->dirPath = (char*)calloc(sizeof(char), strlen(subDirPath) + 1);
        newItem->dirName = (char*)calloc(sizeof(char), strlen(in_dir->d_name) + strlen(subDirPath) + 1);

        strcpy(newItem->dirName, in_dir->d_name);
        strcpy(newItem->dirPath, subDirPath);

        listNode = nodeInit(newItem);
        if(aggregatorManager->directoryDistributor[distributionPointer] == NULL){
            aggregatorManager->directoryDistributor[distributionPointer] = linkedListInit(listNode);
        } else
            push(listNode, aggregatorManager->directoryDistributor[distributionPointer]);


        distributionPointer++;

        strcpy(dirPath, arguments->input_dir);
        strcat(dirPath, "/");
        closedir(SubFD);
    }

    closedir(FD);
    free(dirPath);
    free(subDirPath);
    return aggregatorManager;
}


void printAggregatorManagerDirectoryDistributor(AggregatorServerManager* aggregatorManager, int numOfWorkers){
    Node* currentNode;
    DirListItem* item;
    int counter;
    for (int i = 0; i < numOfWorkers; ++i) {
        counter = 0;
        currentNode = (Node*)aggregatorManager->directoryDistributor[i]->head;
        fprintf(stdout, "---------------worker %d--------------\n", i);
        while (currentNode != NULL){
            item = (DirListItem*)currentNode->item;
            fprintf(stdout, "\tdirectory: %s\n", item->dirName);
            currentNode = currentNode->next;
            counter +=1;
        }
        fprintf(stdout, "worker%d serves: %d directories\n\n", i, counter);
    }
}

bool make_fifo_name_server_client(pid_t workerNum, char *name){
    sprintf(name, "workerFromServer%d", workerNum);
    return true;
}

bool make_fifo_name_client_server(pid_t workerNum, char *name){
    sprintf(name, "workerFromClient%d", workerNum);
    return true;
}

int countFilesInDirectory(DIR *FD){
    struct dirent* in_file;
    int counter = 0;
    while ((in_file = readdir(FD))){
        if (!strcmp(in_file->d_name, "."))
            continue;
        if (!strcmp(in_file->d_name, ".."))
            continue;

        counter += 1;
    }
    rewinddir(FD);
    return counter;
}

FileItem* createFileArray(DIR * FD, DirListItem* item, int arraySize){

    struct dirent* in_file;
    char *subDirPath = malloc(DIR_LEN*sizeof(char));
    FileItem* fileArray = (struct FileItem*)malloc(sizeof(struct FileItem)*arraySize);
    int filePointer = 0;
    char* temp =  malloc(sizeof(char) * DATA_SPACE);

    while ((in_file = readdir(FD))) {

        if (!strcmp(in_file->d_name, "."))
            continue;
        if (!strcmp(in_file->d_name, ".."))
            continue;

        fileArray[filePointer].dateFile =  malloc(sizeof(struct Date));

        fileArray[filePointer].filePath = malloc(sizeof(char) * DIR_LEN);
        fileArray[filePointer].fileName = malloc(sizeof(char) * DIR_LEN);
        fileArray[filePointer].numOfDiseases = 0;

        strcpy(subDirPath, item->dirPath);
        strcat(subDirPath, "/");
        strcat(subDirPath, in_file->d_name);
        strcpy(temp, in_file->d_name);

        fileArray[filePointer].dateFile->day = atoi(strtok(temp, "-"));
        fileArray[filePointer].dateFile->month = atoi(strtok(NULL, "-"));
        fileArray[filePointer].dateFile->year = atoi(strtok(NULL, "-"));

        strcpy(fileArray[filePointer].filePath, subDirPath);
        strcpy(fileArray[filePointer].fileName, in_file->d_name);

        //fprintf(stdout, "%s \n", fileItemsArray[filePointer].filePath);

        filePointer++;
    }
    qsort(fileArray, arraySize, sizeof(FileItem), compare);
    rewinddir(FD);
    free(temp);
    return fileArray;
}


bool sendStatistics(CmdManager* cmdManager) {

    char* messageSize;
    char* message;

    /*write the number of directories that will send stats to follow to fifo*/
    message = calloc(sizeof(char), cmdManager->bufferSize);
    sprintf(message, "%d", cmdManager->numOfDirectories);
    writeInFifoPipe(cmdManager->fd_client_w, message, (cmdManager->bufferSize) + 1);
    free(message);
    /*send statistics*/
    for (int i = 0; i < cmdManager->numOfDirectories; i++) {
        /*write the country*/
        writeInFifoPipe(cmdManager->fd_client_w, cmdManager->fileExplorer[i]->country,(cmdManager->bufferSize) + 1);

        /*write number of files for the country*/
        messageSize = calloc(sizeof(char), cmdManager->bufferSize);
        sprintf(messageSize, "%d", cmdManager->fileExplorer[i]->fileArraySize);
        writeInFifoPipe(cmdManager->fd_client_w, messageSize, (cmdManager->bufferSize)  + 1);
        free(messageSize);
        for (int j = 0; j < cmdManager->fileExplorer[i]->fileArraySize; j++) {
            /*write the file name*/

            writeInFifoPipe(cmdManager->fd_client_w, cmdManager->fileExplorer[i]->fileItemsArray[j].fileName,
                                (cmdManager->bufferSize) + 1);

            /*write number of diseases for the country*/
            messageSize = calloc(sizeof(char), cmdManager->bufferSize);
            sprintf(messageSize, "%d", cmdManager->fileExplorer[i]->fileItemsArray[j].numOfDiseases);
            writeInFifoPipe(cmdManager->fd_client_w, messageSize, (cmdManager->bufferSize)  + 1);
            free(messageSize);
            for (int k = 0; k < cmdManager->fileExplorer[i]->fileItemsArray[j].numOfDiseases; k++) {
                /*write disease*/

                writeInFifoPipe(cmdManager->fd_client_w,
                                    cmdManager->fileExplorer[i]->fileItemsArray[j].fileDiseaseStats[k]->disease,(cmdManager->bufferSize) + 1);
                /*write stats for age ranges*/
                for (int l = 0; l < 4; l++) {
                    message = calloc(sizeof(char), DATA_SPACE + 1);
                    if(l == 0){
                        sprintf(message, "Age range 0-20 years: %d cases", cmdManager->fileExplorer[i]->fileItemsArray[j].fileDiseaseStats[k]->AgeRangeCasesArray[l]);
                    }else if(l == 1){
                        sprintf(message, "Age range 21-40 years: %d cases", cmdManager->fileExplorer[i]->fileItemsArray[j].fileDiseaseStats[k]->AgeRangeCasesArray[l]);
                    }else if(l == 2){
                        sprintf(message, "Age range 41-60 years: %d cases", cmdManager->fileExplorer[i]->fileItemsArray[j].fileDiseaseStats[k]->AgeRangeCasesArray[l]);
                    }else if(l == 3){
                        sprintf(message, "Age range 60+ years: %d cases", cmdManager->fileExplorer[i]->fileItemsArray[j].fileDiseaseStats[k]->AgeRangeCasesArray[l]);
                    }

                    writeInFifoPipe(cmdManager->fd_client_w, message,(cmdManager->bufferSize) + 1);

                    free(message);
                }
                /*end of stat batch*/

                writeInFifoPipe(cmdManager->fd_client_w, "next",(cmdManager->bufferSize) + 1);
            }
        }
    }

    /*send end of transmission message*/
    writeInFifoPipe(cmdManager->fd_client_w, "StatsDone", cmdManager->bufferSize);
    return true;
}


void freeFileItemsArray(FileDiseaseStats* fileDiseaseStats){
    free(fileDiseaseStats->disease);
    free(fileDiseaseStats->AgeRangeCasesArray);
}



bool receiveStats(AggregatorServerManager* aggregatorServerManager, int workerId){
    char *country, *fileName, *disease, *message, *messageSize;
    int numOfDirs, numOfFiles, numOfDiseases;

    /*read per country*/
    message = calloc(sizeof(char), aggregatorServerManager->bufferSize + 1);
    readFromFifoPipe(aggregatorServerManager->workersArray[workerId].fd_client_r, message, (aggregatorServerManager->bufferSize)+1);
    numOfDirs = atoi(message);
    free(message);

    for (int i = 0; i < numOfDirs; i++) {

        /*read actual message from fifo*/
        country = calloc(sizeof(char), (aggregatorServerManager->bufferSize)+1);
        readFromFifoPipe(aggregatorServerManager->workersArray[workerId].fd_client_r, country,(aggregatorServerManager->bufferSize)+1);

        /*read per file*/
        messageSize = calloc(sizeof(char), (aggregatorServerManager->bufferSize) + 1);
        readFromFifoPipe(aggregatorServerManager->workersArray[workerId].fd_client_r, messageSize, (aggregatorServerManager->bufferSize)+1);
        numOfFiles = atoi(messageSize);
        free(messageSize);
        for (int j = 0; j < numOfFiles; j++) {

            /*read actual message from fifo*/
            fileName = calloc(sizeof(char), (aggregatorServerManager->bufferSize)+1);
            readFromFifoPipe(aggregatorServerManager->workersArray[workerId].fd_client_r, fileName,(aggregatorServerManager->bufferSize)+1);

            /*read per disease*/
            messageSize = calloc(sizeof(char), aggregatorServerManager->bufferSize + 1);
            readFromFifoPipe(aggregatorServerManager->workersArray[workerId].fd_client_r, messageSize, (aggregatorServerManager->bufferSize)+1);
            numOfDiseases = atoi(messageSize);
            free(messageSize);
            for (int k = 0; k < numOfDiseases; k++) {
                /*read actual message from fifo*/
                disease = calloc(sizeof(char), (aggregatorServerManager->bufferSize)+1);
                readFromFifoPipe(aggregatorServerManager->workersArray[workerId].fd_client_r, disease,(aggregatorServerManager->bufferSize)+1);

                for (int l = 0; l < 4; l++) {
                    /*read actual message from fifo*/
                    message = calloc(sizeof(char), aggregatorServerManager->bufferSize+1);
                    readFromFifoPipe(aggregatorServerManager->workersArray[workerId].fd_client_r, message,(aggregatorServerManager->bufferSize)+1);
                    free(message);
                }

                /*read actual message from fifo*/
                message = calloc(sizeof(char), (aggregatorServerManager->bufferSize)+1);
                readFromFifoPipe(aggregatorServerManager->workersArray[workerId].fd_client_r, message,(aggregatorServerManager->bufferSize)+1);
                free(message);
                free(disease);
            }
            free(fileName);
        }
        free(country);
    }
    message = calloc(sizeof(char), (aggregatorServerManager->bufferSize)+1);
    readFromFifoPipe(aggregatorServerManager->workersArray[workerId].fd_client_r, message,(aggregatorServerManager->bufferSize)+1);
    free(message);
    return true;
}


AggregatorInputArguments* getAggregatorInputArgs(int argc, char** argv){

    AggregatorInputArguments* arguments =  malloc(sizeof(struct AggregatorInputArguments));
    int numOfArgs = 0;
    if(argc != 7){
        fprintf(stderr, "Invalid number of arguments\nExit...\n");
        exit(1);
    }
    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-i") == 0) {
            arguments->input_dir = malloc(sizeof(char)*254);
            strcpy(arguments->input_dir, argv[i + 1]);
            numOfArgs += 2;
        } else if (strcmp(argv[i], "-w") == 0) {
            arguments->numWorkers = atoi(argv[i + 1]);
            numOfArgs += 2;
        } else if (strcmp(argv[i], "-b") == 0) {
            arguments->bufferSize = atoi(argv[i + 1]);
            if(arguments->bufferSize < 120){
                arguments->bufferSize = 120;
            }
            numOfArgs += 2;
        } else {
            fprintf(stderr, "Unknown option %s\n", argv[i]);
            exit(1);
        }
    }
    if (arguments->input_dir == NULL) {
        fprintf(stdout, "Default file patientRecordsFile loaded...\n");
    }

    return arguments;
}

void DiseaseAggregatorServerManager(AggregatorServerManager* aggregatorServerManager){

    char* command = NULL;
    char* simpleCommand = NULL;
    char* arguments = NULL;
    char* message = NULL;
    char* line = NULL;
    char* answer = NULL;
    size_t length = 0;

    fprintf(stdout,"~$:");
    while (getline(&line, &length, stdin) != EOF){

        simpleCommand = strtok(line, "\n");
        if(simpleCommand == NULL){
            continue;
        }else if(strcmp(simpleCommand, "/help") == 0){
            helpDesc();
        } else if(strcmp(simpleCommand, "/exit") == 0){
            free(line);
            for (int i = 0; i < aggregatorServerManager->numOfWorkers; i++) {
                writeInFifoPipe(aggregatorServerManager->workersArray[i].fd_client_w, simpleCommand, aggregatorServerManager->bufferSize + 1);
            }
        }else if(strcmp(simpleCommand, "/listCountries") == 0){
            listCountries(aggregatorServerManager);
        }else {

            command = strtok(simpleCommand, " ");
            arguments = strtok(NULL, "\n");
            if(strcmp(command, "/diseaseFrequency") == 0 || strcmp(command, "/topk-AgeRanges") == 0 ||
                        strcmp(command, "/searchPatientRecord") == 0 || strcmp(command, "/numPatientAdmissions") == 0
                        || strcmp(command, "/numPatientDischarges") == 0){

                message = calloc(sizeof(char), DATA_SPACE);
                sprintf(message, "%s %s", command, arguments);

                for (int i = 0; i < aggregatorServerManager->numOfWorkers; i++) {
                    writeInFifoPipe(aggregatorServerManager->workersArray[i].fd_client_w, message, aggregatorServerManager->bufferSize + 1 );
                    answer = calloc(sizeof(char), aggregatorServerManager->bufferSize + 1);
                    readFromFifoPipe(aggregatorServerManager->workersArray[i].fd_client_r, answer, aggregatorServerManager->bufferSize + 1);
                    fprintf(stdout, "%s\n", answer);
                }
                fprintf(stdout, "~$:");
                free(message);
            }else{
                fprintf(stdout,"The command you have entered does not exist.\n You can see the "
                               "available commands by hitting /help.\n~$:");
            }
        }
    }

}

void nodeDirListItemDeallock(DirListItem* dirListItem){
    free(dirListItem->dirPath);
    free(dirListItem->dirName);
    free(dirListItem);
}


void freeAggregatorManager(AggregatorServerManager *aggregatorManager){
    Node* listNode;
    for (int i = 0; i < aggregatorManager->numOfWorkers; ++i) {
        listNode = aggregatorManager->directoryDistributor[i]->head;
        while(listNode != NULL){
            aggregatorManager->directoryDistributor[i]->head = aggregatorManager->directoryDistributor[i]->head->next;
            nodeDirListItemDeallock(listNode->item);
            free(listNode);
            listNode = aggregatorManager->directoryDistributor[i]->head;
        }
        free(aggregatorManager->directoryDistributor[i]);
        freeWorkerInfo(aggregatorManager->workersArray[i]);
    }
    free(aggregatorManager->directoryDistributor);
    free(aggregatorManager->workersArray);
    free(aggregatorManager);
}

void freeAggregatorInputArguments(AggregatorInputArguments *aggregatorInputArguments){
    free(aggregatorInputArguments->input_dir);
    free(aggregatorInputArguments);
}

void freeWorkerInfo(WorkerInfo workerInfo){
    free(workerInfo.serverFileName);
}


