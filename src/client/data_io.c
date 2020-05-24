//
// Created by AriannaK97 on 9/3/20.
//
#define  _GNU_SOURCE
#include "../../header/data_io.h"
#include "../../header/diseaseAggregator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/wait.h>

FILE* openFile(char *inputFile){
    FILE *patientRecordsFile;

    patientRecordsFile = fopen(inputFile, "r");

    if (patientRecordsFile == NULL) {

        fprintf(stderr, "File %s, could not be opened. Are you sure this file exists?\n", inputFile);
        exit(1);
    } else {

        fprintf(stdout, "File %s was succesfully opened.\n", inputFile);
    }
    return patientRecordsFile;
}

MonitorInputArguments* getMonitorInputArgs(int argc, char** argv){

    MonitorInputArguments* arguments =  (struct MonitorInputArguments*)malloc(sizeof(struct MonitorInputArguments));
    if(argc != 6){
        fprintf(stderr, "Invalid number of arguments\nExit...\n");
        exit(1);
    }
    for (int i = 1; i < argc; i++) {
        if (i == 1) {
            arguments->bufferSize = atoi(argv[i]);
        } else if (i == 2) {
            arguments->diseaseHashtableNumOfEntries = atoi(argv[i]);
        } else if (i == 3) {
            arguments->countryHashTableNumOfEntries = atoi(argv[i]);

        } else if (i == 4) {
            arguments->bucketSize = atoi(argv[i]);
        } else if (i == 5){
            arguments->input_dir = (char*)malloc(sizeof(char)*DIR_LEN);
            strcpy(arguments->input_dir, argv[i]);
        }
    }

    return arguments;
}


/***
 * Get the max value of either the longest line in file or the buffersize->max lines in file
 * returnVal accepts a value from enum {LINE_LENGTH, BUFFER_SIZE}, returning the adequate
 * value respectively.
 * */
int getMaxFromFile(FILE* patientRecordsFile, int returnVal){
    int c = 0;
    int LINEBUFFERSIZE = 0;
    int maxStringLength = 0;
    int currentStringLength = 0;

    while(c != EOF){
        c = fgetc(patientRecordsFile);
        currentStringLength += 1;
        if(c == '\n'){
            LINEBUFFERSIZE++;
            if(currentStringLength > maxStringLength){  //looking for the longest line in the textfile
                maxStringLength = currentStringLength;
                currentStringLength = 0;
            }
        }
    }
    rewind(patientRecordsFile);     /*return file pointer back to the top of the file*/

    if (c == EOF){
        LINEBUFFERSIZE += 1; //the last line read is not counted
    }

    if (returnVal == LINE_LENGTH) {
        return maxStringLength;
    }else if (returnVal == LINE_BUFFER_SIZE) {
        return LINEBUFFERSIZE;
    }
    return 0;
}


PatientCase* getPatient(char* buffer, FileExplorer* fileExplorer, int fileExplorerPointer, int dirNum){
    const char* delim = " ";
    int tokenCase = 0;
    char* token = NULL;
    PatientCase *newPatient = (struct PatientCase*)malloc(sizeof(struct PatientCase));
    newPatient->entryDate = (struct Date*)malloc(sizeof(struct Date));
    newPatient->exitDate = (struct Date*)malloc(sizeof(struct Date));
    newPatient->entryDate->day = 0;
    newPatient->entryDate->month = 0;
    newPatient->entryDate->year = 0;
    newPatient->exitDate->day = 0;
    newPatient->exitDate->month = 0;
    newPatient->exitDate->year = 0;

    token = strtok(buffer, delim);
    while(tokenCase != 6 && token != NULL){
        if (tokenCase == 0){
            newPatient->recordID = malloc(DATA_SPACE * sizeof(char));
            strcpy(newPatient->recordID, token);
            token = strtok(NULL, delim);
        }else if (tokenCase == 1){
            newPatient->type = malloc(DATA_SPACE * sizeof(char));
            strcpy(newPatient->type, token);
            token = strtok(NULL, delim);
        }else if (tokenCase == 2){
            newPatient->name = malloc(DATA_SPACE*sizeof(char));
            strcpy(newPatient->name, token);
            token = strtok(NULL, delim);
        }else if (tokenCase == 3){
            newPatient->surname = malloc(DATA_SPACE*sizeof(char));
            strcpy(newPatient->surname, token);
            token = strtok(NULL, delim);
        }else if (tokenCase == 4){
            newPatient->virus = malloc(DATA_SPACE*sizeof(char));
            strcpy(newPatient->virus, token);
            token = strtok(NULL, delim);
        }else if (tokenCase == 5){
            newPatient->age = atoi(token);
        }

        tokenCase++;
    }

    if(strcmp(newPatient->recordID, "Hvyfj8") == 0){
        printf("bla\n");
    }

    newPatient->country = malloc(DATA_SPACE*sizeof(char));
    strcpy(newPatient->country, fileExplorer->country);
    if(!setDate(newPatient, fileExplorer->fileArray[dirNum][fileExplorerPointer].fileName)){
        fprintf(stderr, "Date Error!\n");
        free(newPatient);
        return NULL;
    }
    return newPatient;
}

bool setDate(PatientCase *patient, char *buffer){
    bool _ret = true;
    char *temp = malloc(DATA_SPACE*sizeof(char));
    strcpy(temp, buffer);
    if(strcmp(patient->type, "ENTRY")==0){
        patient->entryDate->day = atoi(strtok(temp,"-"));
        patient->entryDate->month = atoi(strtok(NULL, "-"));
        patient->entryDate->year = atoi(strtok(NULL, "-"));
    }else if(strcmp(patient->type, "EXIT")==0){
        patient->exitDate->day = atoi(strtok(temp,"-"));
        patient->exitDate->month = atoi(strtok(NULL, "-"));
        patient->exitDate->year = atoi(strtok(NULL, "-"));
    }else
        _ret = false;
    free(temp);
    return _ret;
}

bool writeEntry(char* buffer, List* patientList, HashTable* diseaseHashTable, HashTable* countryHashTable, int bucketSize, int fileExplorerPointer, int dirNum){
    PatientCase* newPatient;
    Node* newNode;

    newPatient = getPatient(buffer, NULL, 1, 1);
    newNode = nodeInit(newPatient);

    if(patientList->head == NULL){
        patientList = linkedListInit(newNode);
    }else if(!searchListForRecordID(patientList, newPatient->recordID)){
        push(newNode, patientList);
    }
    hashPut(diseaseHashTable, strlen(newPatient->virus), newPatient->virus, bucketSize, newNode);
    hashPut(countryHashTable, strlen(newPatient->country), newPatient->country, bucketSize, newNode);
    return true;
}

CmdManager* initializeStructures(MonitorInputArguments *monitorInputArguments){
    CmdManager* cmdManager = malloc(sizeof(struct CmdManager));

    cmdManager->patientList = NULL;
    cmdManager->input_dir = (char*)malloc(sizeof(char)*DIR_LEN);
    strcpy(cmdManager->input_dir, monitorInputArguments->input_dir);
    cmdManager->countryHashTable = hashCreate(monitorInputArguments->countryHashTableNumOfEntries);
    cmdManager->diseaseHashTable = hashCreate(monitorInputArguments->diseaseHashtableNumOfEntries);
    cmdManager->bucketSize = monitorInputArguments->bucketSize;
    cmdManager->directoryList = NULL;

    return cmdManager;
}

CmdManager* read_input_file(FILE* patientRecordsFile, size_t maxStrLength, CmdManager* cmdManager, FileExplorer* fileExplorer, int fileExplorerPointer, int dirNum){
    char* buffer = malloc(sizeof(char)*maxStrLength);
    PatientCase* newPatient = NULL;
    Node* newNode = NULL;

    while(getline(&buffer, &maxStrLength, patientRecordsFile) >= 0){
        newPatient = getPatient(buffer, fileExplorer, fileExplorerPointer, dirNum);
        if(newPatient != NULL){
            if(strcmp(newPatient->type, "ENTRY")==0){
                newNode = nodeInit(newPatient);
                if(cmdManager->patientList == NULL){
                    cmdManager->patientList = linkedListInit(newNode);
                }else if(!searchListForRecordID(cmdManager->patientList, newPatient->recordID)){
                    push(newNode, cmdManager->patientList);
                }
                hashPut(cmdManager->diseaseHashTable, strlen(newPatient->virus), newPatient->virus, cmdManager->bucketSize, newNode);
                hashPut(cmdManager->countryHashTable, strlen(newPatient->country), newPatient->country, cmdManager->bucketSize, newNode);
                fileExplorer->successfulEntries+=1;
            }else if(cmdManager->patientList!=NULL && strcmp(newPatient->type, "EXIT")==0){
                if(searchNodeForRecordID_ExitDateUpdate(cmdManager->patientList, newPatient->recordID, newPatient->exitDate)) {
                    nodeItemDeallock(newPatient);
                    fileExplorer->successfulEntries += 1;
                }else{
                    nodeItemDeallock(newPatient);
                    fileExplorer->failedEntries+=1;
                    //fprintf(stderr, "Error\n");
                }
            }
        }
    }

    free(buffer);
    //printList(cmdManager->patientList);
    return cmdManager;
}

bool dateInputValidation(Date* entryDate, Date* exitDate){
    if (entryDate->day == exitDate->day && entryDate->month == exitDate->month && entryDate->year < exitDate->year)
        return true;
    if (entryDate->day == exitDate->day && entryDate->month < exitDate->month && entryDate->year <= exitDate->year)
        return true;
    if ((entryDate->day < exitDate->day ||  entryDate->day >= exitDate->day) &&
        (entryDate->month <= exitDate->month || entryDate->month > exitDate->month) &&
        entryDate->year <= exitDate->year)
        return true;
    return false;
}

void deallockFileExplorer(FileExplorer *fileExplorer){
    free(fileExplorer->country);
    free(fileExplorer);
}

CmdManager* read_directory_list(CmdManager* cmdManager){

    Node* node = cmdManager->directoryList->head;
    FILE* entry_file;
    DIR* FD;
    DirListItem* item;
    FileItem* fileArray;
    FileExplorer* fileExplorer;
    int numOfFileInSubDirectory = 0;
    int arraySize;
    int dirNum = 0;

    while (node != NULL) {
        fileExplorer = malloc(sizeof(FileExplorer));
        item = (DirListItem*)node->item;

        /* Scanning the in directory */
        if (NULL == (FD = opendir(item->dirPath))) {
            fprintf(stderr, "Error : Failed to open input directory - %s\n", strerror(errno));
            exit(1);
        }

        arraySize = countFilesInDirectory(FD);
        fileExplorer->country = malloc(sizeof(char) * DIR_LEN);
        fileExplorer->successfulEntries = 0;
        fileExplorer->failedEntries = 0;
        fileExplorer->fileArray = malloc(sizeof(FileItem) * arraySize);
        fileArray = createFileArray(FD, item, arraySize);
        strcpy(fileExplorer->country, item->dirName);
        memcpy(&fileExplorer->fileArray[dirNum], &fileArray, sizeof(FileItem) * arraySize);

        for (int i = 0; i < arraySize-1; i++) {
            entry_file = fopen(fileArray[i].filePath, "r");
            if (entry_file == NULL) {
                fprintf(stderr, "Error : Failed to open entry file %s - %s\n", fileArray[i].filePath, strerror(errno));
                exit(1);
            }

            cmdManager = read_input_file(entry_file, getMaxFromFile(entry_file, LINE_LENGTH), cmdManager,
                                         fileExplorer, i, dirNum);
            fclose(entry_file);
            numOfFileInSubDirectory++;
            printf("list items:%d\n", cmdManager->patientList->itemCount);
        }
        dirNum++;
        node = node->next;
        printf("kjnweofnwnwol");
    }
/*    printf("kjnweofnwnwol");
    if(cmdManager->patientList->itemCount != 0)
        printf("%d\n\n\n\n", cmdManager->patientList->itemCount);
    printList(cmdManager->patientList);*/

    return cmdManager;
}


int compare (const void * a, const void * b){

    int ret;

    FileItem *ia = (FileItem *)a; // casting pointer types
    FileItem *ib = (FileItem *)b;

    ret = compare_dates(ia->dateFile, ib->dateFile);

    return ret;

}
