//
// Created by AriannaK97 on 9/3/20.
//
#define  _GNU_SOURCE
#include "../header/data_io.h"
#include <stdio.h>

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
    if(argc != 9){
        fprintf(stderr, "Invalid number of arguments\nExit...\n");
        exit(1);
    }
    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-p") == 0) {
            arguments->inputFile = malloc(sizeof(char)*254);
            strcpy(arguments->inputFile, argv[i + 1]);
            numOfArgs += 2;
        } else if (strcmp(argv[i], "-h1") == 0) {
            arguments->diseaseHashtableNumOfEntries = atoi(argv[i + 1]);
            numOfArgs += 2;
        } else if (strcmp(argv[i], "-h2") == 0) {
            arguments->countryHashTableNumOfEntries = atoi(argv[i + 1]);
            numOfArgs += 2;
        } else if (strcmp(argv[i], "-b") == 0) {
            if(atoi(argv[i + 1]) >= BUCKET_SIZE){
                arguments->bucketSize = atoi(argv[i + 1]);
                numOfArgs += 2;
            }else{
                fprintf(stderr, "The BUCKET_SIZE you have provided is invalid! Provide a BUCKET_SIZE >= %d\n", BUCKET_SIZE);
                scanf("%ld" ,&arguments->bucketSize);
            }
        } else {
            fprintf(stderr, "Unknown option %s\n", argv[i]);
            exit(1);
        }
    }
    if (arguments->inputFile == NULL) {
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
    int BUFFERSIZE = 0;
    int maxStringLength = 0;
    int currentStringLength = 0;

    while(c != EOF){
        c = fgetc(patientRecordsFile);
        currentStringLength += 1;
        if(c == '\n'){
            BUFFERSIZE++;
            if(currentStringLength > maxStringLength){  //looking for the longest line in the textfile
                maxStringLength = currentStringLength;
                currentStringLength = 0;
            }
        }
    }
    rewind(patientRecordsFile);     /*return file pointer back to the top of the file*/

    if (c == EOF){
        BUFFERSIZE += 1; //the last line read is not counted
    }

    if (returnVal == LINE_LENGTH) {
        return maxStringLength;
    }else if (returnVal == BUFFER_SIZE) {
        return BUFFERSIZE;
    }
    return 0;
}


PatientCase* getPatient(char* buffer){
    const char* delim = " ";
    const char* dateDelim = " -";
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
    while(tokenCase != 12 && token != NULL){
        if (tokenCase == 0){
            newPatient->caseNum = malloc(DATA_SPACE*sizeof(char));
            strcpy(newPatient->caseNum, token);
            token = strtok(NULL, delim);
        } else if (tokenCase == 1){
            newPatient->name = malloc(DATA_SPACE*sizeof(char));
            strcpy(newPatient->name, token);
            token = strtok(NULL, delim);
        }else if (tokenCase == 2){
            newPatient->surname = malloc(DATA_SPACE*sizeof(char));
            strcpy(newPatient->surname, token);
            token = strtok(NULL, delim);
        }else if (tokenCase == 3){
            newPatient->virus = malloc(DATA_SPACE*sizeof(char));
            strcpy(newPatient->virus, token);
            token = strtok(NULL, delim);
        }else if (tokenCase == 4){
            newPatient->country = malloc(DATA_SPACE*sizeof(char));
            strcpy(newPatient->country, token);
            token = strtok(NULL, dateDelim);
        }else if (tokenCase == 5){
            newPatient->entryDate->day = atoi(token);
            token = strtok(NULL, dateDelim);
        }else if (tokenCase == 6){
            newPatient->entryDate->month = atoi(token);
            token = strtok(NULL, dateDelim);
        }else if (tokenCase == 7){
            newPatient->entryDate->year = atoi(token);
            token = strtok(NULL, dateDelim);
            /*if there is no next token for the export date we infer that there is no such
             * and assign the arbitrary value -1*/
            if(token == NULL){
                newPatient->exitDate->day = 0;
                newPatient->exitDate->month = 0;
                newPatient->exitDate->year = 0;
                tokenCase = 9;
            }
        }else if (tokenCase == 8){
            newPatient->exitDate->day = atoi(token);
            token = strtok(NULL, dateDelim);
        }else if (tokenCase == 9){
            newPatient->exitDate->month = atoi(token);
            token = strtok(NULL, dateDelim);
        }else if (tokenCase == 10){
            newPatient->exitDate->year = atoi(token);
            if(!dateInputValidation(newPatient->entryDate, newPatient->exitDate)){
                fprintf(stderr, "New record discarded\n Invalid dates for patient %s : importDate: %d-%d-%d | exportDate: %d-%d-%d\n",
                        newPatient->caseNum, newPatient->entryDate->day, newPatient->entryDate->month, newPatient->entryDate->year,
                        newPatient->exitDate->day, newPatient->exitDate->month, newPatient->exitDate->year);
                return NULL;
            }
            token = strtok(NULL, dateDelim);
        }
        tokenCase++;
    }
    return newPatient;
}

bool writeEntry(char* buffer, List* patientList, HashTable* diseaseHashTable, HashTable* countryHashTable, int bucketSize){
    PatientCase* newPatient;
    Node* newNode;

    newPatient = getPatient(buffer);
    newNode = nodeInit(newPatient);

    if(patientList->head == NULL){
        patientList = linkedListInit(newNode);
    }else if(!searchListForRecordID(patientList, newPatient->caseNum)){
        push(newNode, patientList);
    }
    hashPut(diseaseHashTable, strlen(newPatient->virus), newPatient->virus, bucketSize, newNode);
    hashPut(countryHashTable, strlen(newPatient->country), newPatient->country, bucketSize, newNode);
    return true;
}

CmdManager* read_input_file(FILE* patientRecordsFile, size_t maxStrLength, int diseaseHashtableNumOfEntries,
                            int countryHashTableNumOfEntries, size_t bucketSize){
    char* buffer = malloc(sizeof(char)*maxStrLength);
    CmdManager* cmdManager = malloc(sizeof(struct CmdManager));
    List* patientList = NULL;
    PatientCase* newPatient = NULL;
    Node* newNode = NULL;

    HashTable* diseaseHashTable = hashCreate(diseaseHashtableNumOfEntries);
    HashTable* countryHashTable = hashCreate(countryHashTableNumOfEntries);

    while(getline(&buffer, &maxStrLength, patientRecordsFile) >= 0){

        newPatient = getPatient(buffer);
        if(newPatient != NULL){
            newNode = nodeInit(newPatient);
            if(patientList == NULL){
                patientList = linkedListInit(newNode);
            }else if(!searchListForRecordID(patientList, newPatient->caseNum)){
                push(newNode, patientList);
            }
            hashPut(diseaseHashTable, strlen(newPatient->virus), newPatient->virus, bucketSize, newNode);
            hashPut(countryHashTable, strlen(newPatient->country), newPatient->country, bucketSize, newNode);
        }
    }


    /**
     * Put the needed structures to command manager
     * */
    cmdManager->patientList = patientList;
    cmdManager->countryHashTable = countryHashTable;
    cmdManager->diseaseHashTable = diseaseHashTable;
    cmdManager->bucketSize = bucketSize;

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