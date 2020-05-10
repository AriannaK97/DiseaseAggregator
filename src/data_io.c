//
// Created by AriannaK97 on 9/3/20.
//
#define  _GNU_SOURCE
#include "../header/data_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

CmdManager* readDirectoryFiles_And_PopulateAggregator(InputArguments* arguments, CmdManager* cmdManager){
    DIR* FD;
    DIR* SubFD;
    struct dirent* in_dir;
    struct dirent* in_file;
    FILE    *entry_file;
    char *dirPath = malloc(DIR_LEN*sizeof(char));;
    char *subDirPath = malloc(DIR_LEN*sizeof(char));;

    cmdManager->fileExplorer =  malloc(sizeof(struct FileExplorer));
    cmdManager->fileExplorer->date = malloc(DATA_SPACE*sizeof(char));
    cmdManager->fileExplorer->country = malloc(DATA_SPACE*sizeof(char));
    cmdManager->fileExplorer->failedEntries = 0;
    cmdManager->fileExplorer->successfulEntries = 0;

    /* Scanning the in directory */
    if (NULL == (FD = opendir (arguments->input_dir))){
        fprintf(stderr, "Error : Failed to open input directory - %s\n", strerror(errno));
        exit(1);
    }
    strcpy(dirPath, arguments->input_dir);
    strcat(dirPath, "/");
    while ((in_dir = readdir(FD))){
        /* On linux/Unix we don't want current and parent directories
         * On windows machine too, thanks Greg Hewgill
         */
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
        while ((in_file = readdir(SubFD))) {
            /* On linux/Unix we don't want current and parent directories
             * On windows machine too, thanks Greg Hewgill
             */
            if (!strcmp (in_file->d_name, "."))
                continue;
            if (!strcmp (in_file->d_name, ".."))
                continue;
            strcat(subDirPath, "/");
            strcat(subDirPath, in_file->d_name);
            entry_file = fopen(subDirPath, "r");
            if (entry_file == NULL) {
                fprintf(stderr, "Error : Failed to open entry file %s - %s\n",subDirPath, strerror(errno));
                exit(1);
            }
            strcpy(cmdManager->fileExplorer->country,in_dir->d_name);
            strcpy(cmdManager->fileExplorer->date,in_file->d_name);
            cmdManager = read_input_file(entry_file, getMaxFromFile(entry_file, LINE_LENGTH), cmdManager, cmdManager->fileExplorer);
            fclose(entry_file);
            strcpy(subDirPath, dirPath);
            strcat(subDirPath, in_dir->d_name);
        }
        strcpy(dirPath, arguments->input_dir);
        strcat(dirPath, "/");
        closedir(SubFD);
    }

    fprintf(stdout, "Failed entries: %d\nSuccessful Entries: %d\n"
                    "total Entries: %d\n", cmdManager->fileExplorer->failedEntries, cmdManager->fileExplorer->successfulEntries,
            cmdManager->fileExplorer->failedEntries+cmdManager->fileExplorer->successfulEntries);

    closedir(FD);
    free(dirPath);
    free(subDirPath);
    return cmdManager;
}

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

InputArguments* getInputArgs(int argc, char** argv){

    InputArguments* arguments =  malloc(sizeof(struct InputArguments));
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


PatientCase* getPatient(char* buffer, FileExplorer* fileExplorer){
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
    newPatient->country = malloc(DATA_SPACE*sizeof(char));
    strcpy(newPatient->country, fileExplorer->country);
    if(!setDate(newPatient, fileExplorer->date)){
        fprintf(stderr, "Date Error!\n");
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

bool writeEntry(char* buffer, List* patientList, HashTable* diseaseHashTable, HashTable* countryHashTable, int bucketSize){
    PatientCase* newPatient;
    Node* newNode;

    newPatient = getPatient(buffer, NULL);
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

CmdManager* initializeStructures(int diseaseHashtableNumOfEntries, int countryHashTableNumOfEntries, size_t bucketSize){
    CmdManager* cmdManager = malloc(sizeof(struct CmdManager));
    List* patientList = NULL;

    HashTable* diseaseHashTable = hashCreate(diseaseHashtableNumOfEntries);
    HashTable* countryHashTable = hashCreate(countryHashTableNumOfEntries);
    /**
     * Put the needed structures to command manager
     **/
    cmdManager->patientList = patientList;
    cmdManager->countryHashTable = countryHashTable;
    cmdManager->diseaseHashTable = diseaseHashTable;
    cmdManager->bucketSize = bucketSize;

    return cmdManager;
}

CmdManager* read_input_file(FILE* patientRecordsFile, size_t maxStrLength, CmdManager* cmdManager, FileExplorer* fileExplorer){
    char* buffer = malloc(sizeof(char)*maxStrLength);
    PatientCase* newPatient = NULL;
    Node* newNode = NULL;

    while(getline(&buffer, &maxStrLength, patientRecordsFile) >= 0){
        newPatient = getPatient(buffer, fileExplorer);
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
/*                    fprintf(stderr, "Invalid Patient type EXIT - Discarded\n");
                    fprintf(stdout, "Continuing...\n");*/
                }
            }
        }
    }

    free(buffer);

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
    free(fileExplorer->date);
    free(fileExplorer);
}