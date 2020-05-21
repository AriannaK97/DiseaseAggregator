//
// Created by linuxuser on 21/5/20.
//

#include "../header/diseaseAggregator.h"


AggregatorManager* readDirectoryFiles(InputArguments* arguments){
    DIR* FD;
    DIR* SubFD;
    struct dirent* in_dir;
    FILE *entry_file;
    char *dirPath = malloc(DIR_LEN*sizeof(char));
    char *subDirPath = malloc(DIR_LEN*sizeof(char));
    AggregatorManager* aggregatorManager;
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

    aggregatorManager = malloc(sizeof(AggregatorManager));
    /*array of lists - each list holds the directories assigned to each worker*/
    aggregatorManager->directoryDistributor = malloc(sizeof(struct List)*arguments->numWorkers);

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

        newNode = malloc(sizeof(struct DirListItem));
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


void printAggregatorManagerDirectoryDistributor(AggregatorManager* aggregatorManager, int numOfWorkers){
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
        fprintf(stdout, "Worker serves: %d directories\n\n", counter);
    }
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
    FileItem* fileArray = malloc(sizeof(struct FileItem)*arraySize);
    int filePointer = 0;
    char* temp =  malloc(sizeof(char) * DATA_SPACE);

    while ((in_file = readdir(FD))) {

        if (!strcmp(in_file->d_name, "."))
            continue;
        if (!strcmp(in_file->d_name, ".."))
            continue;

        fileArray[filePointer].dateFile =  malloc(sizeof(struct Date));

        fileArray[filePointer].filePath = malloc(sizeof(char) * DIR_LEN);
        fileArray[filePointer].fileName = malloc(sizeof(char) * DATA_SPACE);

        strcpy(subDirPath, item->dirPath);
        strcat(subDirPath, "/");
        strcat(subDirPath, in_file->d_name);
        strcpy(temp, in_file->d_name);

        fileArray[filePointer].dateFile->day = atoi(strtok(temp, "-"));
        fileArray[filePointer].dateFile->month = atoi(strtok(NULL, "-"));
        fileArray[filePointer].dateFile->year = atoi(strtok(NULL, "-"));

        strcpy(fileArray[filePointer].filePath, subDirPath);
        strcpy(fileArray[filePointer].fileName, in_file->d_name);

        filePointer++;
    }
    qsort(fileArray, arraySize, sizeof(FileItem), compare);
    rewinddir(FD);
    free(temp);
    return fileArray;
}

CmdManager* read_directory_list(List* fileList){

    Node* node = fileList->head;
    FILE* entry_file;
    DIR* FD;
    DirListItem* item;
    FileItem* fileArray;
    struct dirent* in_file;
    int numOfFileInSubDirectory = 0;
    int arraySize;
    int dirNum = 0;
    char *subDirPath = malloc(DIR_LEN*sizeof(char));
    CmdManager* cmdManager;
    cmdManager = initializeStructures(DISEASE_HT_Entries_NUM, COUNTRY_HT_Entries_NUM, BUCKET_SIZE, fileList->itemCount);

    while (node != NULL) {
        item = (DirListItem*)node->item;
        /* Scanning the in directory */
        if (NULL == (FD = opendir(item->dirPath))) {
            fprintf(stderr, "Error : Failed to open input directory - %s\n", strerror(errno));
            exit(1);
        }

        arraySize = countFilesInDirectory(FD);
        fileArray = createFileArray(FD, item, arraySize);

        strcpy(cmdManager->directoryExplorer->country, item->dirName);
        memcpy(&cmdManager->directoryExplorer->fileArray[dirNum], &fileArray, sizeof(FileItem) * arraySize);

        for (int i = 0; i < arraySize; i++) {

            entry_file = fopen(fileArray[i].filePath, "r");
            if (entry_file == NULL) {
                fprintf(stderr, "Error : Failed to open entry file %s - %s\n", subDirPath, strerror(errno));
                exit(1);
            }

            cmdManager = read_input_file(entry_file, getMaxFromFile(entry_file, LINE_LENGTH), cmdManager,
                                         cmdManager->directoryExplorer, i, dirNum);
            fclose(entry_file);
            numOfFileInSubDirectory++;
        }
        dirNum++;
        node = node->next;
    }
    return cmdManager;
}


int compare (const void * a, const void * b){

    int ret;

    FileItem *ia = (FileItem *)a; // casting pointer types
    FileItem *ib = (FileItem *)b;

    ret = compare_dates(ia->dateFile, ib->dateFile);

    return ret;

}
