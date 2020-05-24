//
// Created by linuxuser on 21/5/20.
//

#include "../../header/diseaseAggregator.h"


AggregatorServerManager* readDirectoryFiles(AggregatorInputArguments* arguments){
    DIR* FD;
    DIR* SubFD;
    struct dirent* in_dir;
    char *dirPath = (char*)malloc(DIR_LEN*sizeof(char));
    char *subDirPath = (char*)malloc(DIR_LEN*sizeof(char));
    AggregatorServerManager* aggregatorManager;
    int distributionPointer = 0;
    int listInitiatorFlag = 0;
    DirListItem* newNode = NULL;

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

    aggregatorManager = (struct AggregatorServerManager*)malloc(sizeof(struct AggregatorServerManager));
    /*array of lists - each list holds the directories assigned to each worker*/
    aggregatorManager->directoryDistributor = (struct List**)malloc(sizeof(struct List*)*arguments->numWorkers);

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

        newNode = (struct DirListItem*)malloc(sizeof(struct DirListItem));
        newNode->dirPath = (char*)calloc(sizeof(char), DIR_LEN);
        newNode->dirName = (char*)calloc(sizeof(char), DIR_LEN);

        strcpy(newNode->dirName, in_dir->d_name);
        strcpy(newNode->dirPath, subDirPath);


        if(listInitiatorFlag == 0){
            aggregatorManager->directoryDistributor[distributionPointer] = linkedListInit(nodeInit(newNode));
        } else
            push(nodeInit(newNode), aggregatorManager->directoryDistributor[distributionPointer]);

        if(distributionPointer == arguments->numWorkers-1){
            listInitiatorFlag = 1;
        }

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

bool make_fifo_name(pid_t workerNum, char *name, size_t name_max){
    sprintf(name, "worker%d", workerNum);
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

        strcpy(subDirPath, item->dirPath);
        strcat(subDirPath, "/");
        strcat(subDirPath, in_file->d_name);
        strcpy(temp, in_file->d_name);

        fileArray[filePointer].dateFile->day = atoi(strtok(temp, "-"));
        fileArray[filePointer].dateFile->month = atoi(strtok(NULL, "-"));
        fileArray[filePointer].dateFile->year = atoi(strtok(NULL, "-"));

        strcpy(fileArray[filePointer].filePath, subDirPath);
        strcpy(fileArray[filePointer].fileName, in_file->d_name);

        //fprintf(stdout, "%s \n", fileArray[filePointer].filePath);

        filePointer++;
    }
    qsort(fileArray, arraySize, sizeof(FileItem), compare);
    rewinddir(FD);
    free(temp);
    return fileArray;
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
            if(atoi(argv[i + 1]) >= BUFFER_SIZE){
                arguments->bufferSize = atoi(argv[i + 1]);
                numOfArgs += 2;
            }else{
                fprintf(stderr, "The BUCKET_SIZE you have provided is invalid! Provide a BUCKET_SIZE >= %d\n", BUCKET_SIZE);
                scanf("%ld" ,&arguments->bufferSize);
            }
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


