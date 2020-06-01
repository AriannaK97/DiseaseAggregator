//
// Created by AriannaK97 on 12/3/20.
//
#define  _GNU_SOURCE
#include "../../header/command_lib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../header/structs.h"
#include "../../header/hashTable.h"
#include "../../header/diseaseAggregator.h"
#include "../../header/communication.h"

/**
 * Prints every country along with its Worker's processID. It is usefull in case we want to add new files
 * for a certain country, and the user needs to find out which is the worker'r processID to send the necessary
 * signal to the certain worker.
 * */
void listCountries(AggregatorServerManager* aggregatorServerManager){
    Node* currentNode = NULL;
    DirListItem* item = NULL;
    for (int i = 0; i < aggregatorServerManager->numOfWorkers; ++i) {
        currentNode = (Node*)aggregatorServerManager->directoryDistributor[i]->head;
        while (currentNode != NULL){
            item = currentNode->item;
            fprintf(stdout, "%s %d\n", item->dirName, aggregatorServerManager->workersArray[i].workerPid);
            currentNode = currentNode->next;
        }
    }
    fprintf(stdout, "\n~$:");
}

/**
 * If the argument [country] is not define, the application prints for the virusName
 * defined the number of the diseased monitored in the system during the specified
 * period between [date1, date2]. If [country] is defined, the application prints
 * the number of the diseased in this [country] for the specified period.
 * Cmd Args: virusName date1 date2 [country]
 * */
void diseaseFrequency(CmdManager* manager, char* virusName, Date* date1, Date* date2, char* country){

    HashElement iterator = hashITERATOR(manager->diseaseHashTable);
    iterator.date1 = date1;
    iterator.date2 = date2;
    iterator.virus = virusName;
    if(country == NULL) {
        while (hashIterateValues(&iterator, COUNT_ALL_BETWEEN_DATES_WITH_VIRUS) != NULL);
        char* message = calloc(sizeof(char), manager->bufferSize + 1);
        sprintf(message, "%d", iterator.counter);
        writeInFifoPipe(manager->fd_client_w, message, manager->bufferSize + 1);
    }else {
        iterator.country = country;
        while (hashIterateValues(&iterator, COUNT_ALL_BETWEEN_DATES_WITH_VIRUS_AND_COUNTRY) != NULL);
        char* message = calloc(sizeof(char), manager->bufferSize + 1);
        sprintf(message, "%d", iterator.counter);
        writeInFifoPipe(manager->fd_client_w, message, manager->bufferSize + 1);
    }
    //fprintf(stdout, "\n~$:");
}


/**
 * For the specified country and disease the application prints the age categories with the top k
 * number of diseased cases of the given disease during the period date1, date2, if that is specified.
 * Cmd Args: k country disease date1, date2
 * */
void topk_AgeRanges(CmdManager* manager, int k, char* country, char* disease , Date* date1, Date* date2){
    HashElement iterator = hashITERATOR(manager->diseaseHashTable);
    char* message = calloc(sizeof(char), manager->bufferSize + 1);
    int* ageRangeCasesArray = calloc(sizeof(int), 4);
    float total = 0;
    iterator.country = country;
    iterator.virus = disease;
    iterator.date1 = date1;
    iterator.date2 = date2;
    while (hashIterateValues(&iterator, GET_HEAP_NODES_AGE_RANGE_DATES) != NULL);

    if(iterator.AgeRangeNodes == NULL || iterator.AgeRangeNodes->head == NULL){
        //fprintf(stdout, "There are no countries with cases of %s\n~$:", disease);
        sprintf(message, "null");
        writeInFifoPipe(manager->fd_client_w, message, manager->bufferSize + 1);
        //freeHeapTree(maxHeap);
    }else{
        Node* currentNode = iterator.AgeRangeNodes->head;
        AgeRangeStruct* item;
        while(currentNode != NULL){
            item = currentNode->item;
            total += item->dataSum;
            if (item->data <= 20){
                ageRangeCasesArray[0] = item->dataSum;
            }else if(item->data <= 40){
                ageRangeCasesArray[1] = item->dataSum;
            }else if(item->data <= 60){
                ageRangeCasesArray[2] = item->dataSum;
            }else if(item->data <= 120){
                ageRangeCasesArray[3] = item->dataSum;
            }
            currentNode = currentNode->next;
        }
        if(k > 4){
            k = 4;
        }

        int i = 0;
        while (i < k){
            char *temp = calloc(sizeof(char), 12);
            if (i==0){
                sprintf(temp, "0-20: %.f%%\n", (float)ageRangeCasesArray[i]/total*100);
            }else if(i==1){
                sprintf(temp, "21-40: %.f%%\n", (float)ageRangeCasesArray[i]/total*100);
            }else if(i==2){
                sprintf(temp, "41-60: %.f%%\n", (float)ageRangeCasesArray[i]/total*100);
            }else if(i==3){
                sprintf(temp, "60+: %.f%%\n", (float)ageRangeCasesArray[i]/total*100);
            }
            strcat(message, temp);
            i++;
            free(temp);
        }
        writeInFifoPipe(manager->fd_client_w, message, manager->bufferSize + 1);
    }
}

/**
 * Search the record in system with the given recordID
 * Cmd Args: recordID
 * */
void searchPatientRecord(CmdManager* manager, char* recordID){
    PatientCase* patient;
    char* message = calloc(sizeof(char), manager->bufferSize + 1);
    patient = getPatientFromList(manager->patientList, recordID);
    if(patient == NULL){
        sprintf(message,"null");
    }else {
        if(patient->exitDate->day == 0){
            sprintf(message,"%s %s %s %s %d %d-%d-%d --\n", patient->recordID, patient->name, patient->surname, patient->virus,
                    patient->age, patient->entryDate->day, patient->entryDate->month, patient->entryDate->year);
        } else{
            sprintf(message,"%s %s %s %s %d %d-%d-%d %d-%d-%d\n", patient->recordID, patient->name, patient->surname, patient->virus,
                    patient->age, patient->entryDate->day, patient->entryDate->month, patient->entryDate->year,
                    patient->exitDate->day, patient->exitDate->month, patient->exitDate->year);
        }

    }
    writeInFifoPipe(manager->fd_client_w, message, manager->bufferSize + 1);
}


/**
 * If the argument [country] is given, the application prints the number of the
 * patients hospitalised from the specified country with the specified disease during
 * the period between the given dates.
 * If the [country] is not specified the application prints for all the countries
 * in the system the records of all the patients that are hospitalised with
 * the specified disease, during the period between the given dates.
 * Cmd Args: disease date1 date2 [country]
 * */
void numPatientAdmissions(CmdManager* manager, char* disease, Date* date1, Date* date2, char* country){
    int countryExists = false;
    char* message = calloc(sizeof(char), manager->bufferSize + 1);
    char* patientsNumStr = calloc(sizeof(char), 10);
    int patientsNum = 0;
    if(country != NULL){
        HashElement iterator = hashITERATOR(manager->countryHashTable);
        unsigned int h = hash(strlen(country)) % manager->countryHashTable->capacity;
        Bucket* bucket = manager->countryHashTable->table[h];
        iterator.date1 = date1;
        iterator.date2 = date2;
        iterator.virus = disease;
        iterator.country = country;
        if(bucket != NULL){
            while (bucket != NULL){
                for(int i = 0; i < bucket->numOfEntries; i++){
                    if(strcmp(country, bucket->entry[i].data)==0){
                        patientsNum = countPatients_BetweenDates(bucket->entry[i].tree, COUNT_HOSPITALISED_BETWEEN_DATES_WITH_DISEASE_AND_COUNTRY, &iterator);
                        if(strlen(bucket->entry[i].data)!=0){
                            strcat(message, country);
                            sprintf(patientsNumStr, " %d\n", patientsNum);
                            strcat(message, patientsNumStr);
                        }
                        countryExists = 1;
                        break;
                    }
                }
                if(countryExists)
                    break;
                bucket = bucket->next;
            }
            if(!countryExists || patientsNum == 0){
                sprintf(message, "null");
            }

        }else{
            sprintf(message, "null");
        }
        writeInFifoPipe(manager->fd_client_w, message, manager->bufferSize + 1);
        free(message);
    }else{
        HashElement iterator = hashITERATOR(manager->countryHashTable);
        int totalCounted = 0;
        iterator.date1 = date1;
        iterator.date2 = date2;
        iterator.virus = disease;
        bool foundAnswer = false;
        for (unsigned int h = 0; h < manager->countryHashTable->capacity; h++ ){
            Bucket* bucket = manager->countryHashTable->table[h];
            if(bucket != NULL){
                while (bucket != NULL){
                    for(int i = 0; i < bucket->numOfEntries; i++){
                        patientsNum = countPatients_BetweenDates(bucket->entry[i].tree, COUNT_HOSPITALISED_BETWEEN_DATES_WITH_DISEASE, &iterator);
                        if(strlen(bucket->entry[i].data)!=0 && patientsNum != 0) {
                            strcat(message, bucket->entry[i].data);
                            sprintf(patientsNumStr, " %d\n", patientsNum);
                            strcat(message, patientsNumStr);
                            foundAnswer = true;
                        }
                        totalCounted += patientsNum;
                    }
                    bucket = bucket->next;
                }
            }
        }
        if (!foundAnswer){
            sprintf(message, "null");
        }
        writeInFifoPipe(manager->fd_client_w, message, manager->bufferSize + 1);
        free(message);
    }
}

void numPatientDischarges(CmdManager* manager, char* disease, Date* date1, Date* date2, char* country){
    char* message = calloc(sizeof(char), manager->bufferSize + 1);
    char* patientsNumStr = calloc(sizeof(char), 10);
    int countryExists = false;
    int patientsNum = 0;
    bool foundAnswer = false;
    if(country != NULL){
        HashElement iterator = hashITERATOR(manager->countryHashTable);
        unsigned int h = hash(strlen(country)) % manager->countryHashTable->capacity;
        Bucket* bucket = manager->countryHashTable->table[h];
        iterator.date1 = date1;
        iterator.date2 = date2;
        iterator.virus = disease;
        iterator.country = country;
        if(bucket != NULL){
            while (bucket != NULL){
                for(int i = 0; i < bucket->numOfEntries; i++){
                    if(strcmp(country, bucket->entry[i].data)==0){
                        patientsNum = countPatients_BetweenDates(bucket->entry[i].tree, COUNT_HOSPITALISED_BETWEEN_DATES_WITH_DISEASE_AND_COUNTRY_EXIT, &iterator);
                        if(strlen(bucket->entry[i].data)!=0){
                            strcat(message, country);
                            sprintf(patientsNumStr, " %d\n", patientsNum);
                            strcat(message, patientsNumStr);
                        }
                        countryExists = 1;
                        break;
                    }
                }
                if(countryExists)
                    break;
                bucket = bucket->next;
            }
            if(!countryExists || patientsNum == 0){
                sprintf(message, "null");
            }
        }else
            sprintf(message, "null");
        writeInFifoPipe(manager->fd_client_w, message, manager->bufferSize + 1);
        free(message);
    }else{
        HashElement iterator = hashITERATOR(manager->countryHashTable);
        int totalCounted = 0;
        iterator.date1 = date1;
        iterator.date2 = date2;
        iterator.virus = disease;
        for (unsigned int h = 0; h < manager->countryHashTable->capacity; h++ ){
            Bucket* bucket = manager->countryHashTable->table[h];
            if(bucket != NULL){
                while (bucket != NULL){
                    for(int i = 0; i < bucket->numOfEntries; i++){
                        patientsNum = countPatients_BetweenDates(bucket->entry[i].tree, COUNT_HOSPITALISED_BETWEEN_DATES_WITH_DISEASE_EXIT, &iterator);
                        if(strlen(bucket->entry[i].data)!=0 && patientsNum != 0) {
                            strcat(message, bucket->entry[i].data);
                            sprintf(patientsNumStr, " %d\n", patientsNum);
                            strcat(message, patientsNumStr);
                            foundAnswer = true;
                        }
                        totalCounted += patientsNum;
                    }
                    bucket = bucket->next;
                }
            }
        }
        if (!foundAnswer){
            sprintf(message, "null");
        }
        writeInFifoPipe(manager->fd_client_w, message, manager->bufferSize + 1);
        free(message);
    }
}

/**
 * Exit from the application - Memory dialloccation
 * */
void exitMonitor(CmdManager* manager){
    fprintf(stdout, "exiting\n");
    //fprintf(stdout, "Destroying disease HashTable...\n");
    freeHashTable(manager->diseaseHashTable);

    //fprintf(stdout, "Destroying country HashTable...\n");
    freeHashTable(manager->countryHashTable);

    //fprintf(stdout, "Destroy patient list...\n");
    listMemoryDeallock(manager->patientList);

    //deallockFileExplorer(manager->directoryExplorer);
    free(manager);

    exit(0);
}


void commandServer(CmdManager* manager) {
    char *command = NULL;
    char *simpleCommand = NULL;
    char *line = calloc(sizeof(char), (manager->bufferSize) + 1);
    int reader;
    //size_t length = 0;

    do{
    //while(getline(&line, &length, stdin) != EOF){

        reader = read(manager->fd_client_r, line, manager->bufferSize + 1);
        if (reader < 0) {
            break;
        }
        simpleCommand = strtok(line, "\n");
        if (simpleCommand == NULL) {
            return;
        } else if (strcmp(simpleCommand, "/help") == 0) {
            helpDesc();
        } else if (strcmp(simpleCommand, "/exit") == 0) {
            free(line);
            exitMonitor(manager);
        } else {

            command = strtok(simpleCommand, " ");

            if (strcmp(command, "/listCountries") == 0) {

                //listCountries(manager);

            } else if (strcmp(command, "/diseaseFrequency") == 0) {
                Date *date1;
                Date *date2;
                date1 = malloc(sizeof(struct Date));
                date2 = malloc(sizeof(struct Date));

                char *virusName = strtok(NULL, " ");   //virus
                char *arg2 = strtok(NULL, " ");   //date1
                char *arg3 = strtok(NULL, " ");   //date2
                char *country = strtok(NULL, " ");

                date1->day = atoi(strtok(arg2, "-"));
                date1->month = atoi(strtok(NULL, "-"));
                date1->year = atoi(strtok(NULL, "-"));
                date2->day = atoi(strtok(arg3, "-"));
                date2->month = atoi(strtok(NULL, "-"));
                date2->year = atoi(strtok(NULL, "-"));

                if (country != NULL) {
                    diseaseFrequency(manager, virusName, date1, date2, country);
                } else
                    diseaseFrequency(manager, virusName, date1, date2, NULL);

                free(date1);
                free(date2);

            } else if (strcmp(command, "/topk-AgeRanges") == 0) {

                int k = atoi(strtok(NULL, " "));
                char *country = strtok(NULL, " ");
                char *disease = strtok(NULL, " ");
                char *arg3 = strtok(NULL, " ");
                char *arg4 = strtok(NULL, " ");

                if (arg3 != NULL && arg4 != NULL) {
                    Date *date1 = malloc(sizeof(struct Date));
                    Date *date2 = malloc(sizeof(struct Date));
                    date1->day = atoi(strtok(arg3, "-"));
                    date1->month = atoi(strtok(NULL, "-"));
                    date1->year = atoi(strtok(NULL, "-"));
                    date2->day = atoi(strtok(arg4, "-"));
                    date2->month = atoi(strtok(NULL, "-"));
                    date2->year = atoi(strtok(NULL, "-"));
                    topk_AgeRanges(manager, k, country, disease, date1, date2);
                    free(date1);
                    free(date2);

                } else if (arg3 == NULL || arg4 == NULL) {
                    fprintf(stderr, "Missing date space. Please try again...\n~$:");
                }

            } else if (strcmp(command, "/searchPatientRecord") == 0) {

                char *recordID = strtok(NULL, "\n");
                searchPatientRecord(manager, recordID);

            } else if (strcmp(command, "/numPatientAdmissions") == 0) {

                Date *date1;
                Date *date2;
                date1 = malloc(sizeof(struct Date));
                date2 = malloc(sizeof(struct Date));

                char *virusName = strtok(NULL, " ");   //virus
                char *arg2 = strtok(NULL, " ");   //date1
                char *arg3 = strtok(NULL, " ");   //date2
                char *country = strtok(NULL, " ");

                date1->day = atoi(strtok(arg2, "-"));
                date1->month = atoi(strtok(NULL, "-"));
                date1->year = atoi(strtok(NULL, "-"));
                date2->day = atoi(strtok(arg3, "-"));
                date2->month = atoi(strtok(NULL, "-"));
                date2->year = atoi(strtok(NULL, "-"));

                if (country != NULL) {
                    numPatientAdmissions(manager, virusName, date1, date2, country);
                } else
                    numPatientAdmissions(manager, virusName, date1, date2, NULL);

                free(date1);
                free(date2);

            } else if (strcmp(command, "/numPatientDischarges") == 0) {
                Date *date1;
                Date *date2;
                date1 = malloc(sizeof(struct Date));
                date2 = malloc(sizeof(struct Date));

                char *virusName = strtok(NULL, " ");   //virus
                char *arg2 = strtok(NULL, " ");   //date1
                char *arg3 = strtok(NULL, " ");   //date2
                char *country = strtok(NULL, " ");

                date1->day = atoi(strtok(arg2, "-"));
                date1->month = atoi(strtok(NULL, "-"));
                date1->year = atoi(strtok(NULL, "-"));
                date2->day = atoi(strtok(arg3, "-"));
                date2->month = atoi(strtok(NULL, "-"));
                date2->year = atoi(strtok(NULL, "-"));

                if (country != NULL) {
                    numPatientDischarges(manager, virusName, date1, date2, country);
                } else
                    numPatientDischarges(manager, virusName, date1, date2, NULL);

                free(date1);
                free(date2);
            }
        }
    //}
    }while(reader > 0 );
}



/**
 * Help desc for all the actions of the application and how to call them.
 * */
void helpDesc(){
    fprintf(stdout,"/listCountries\n\n"
                   "/diseaseFrequency virusName date1 date2 [country]\n\n"
                   "/topk-AgeRanges k country disease date1 date2\n\n"
                   "/searchPatientRecord recordID\n\n"
                   "/numPatientAdmissions disease date1 date2 [country]\n\n"
                   "/numPatientDischarges disease date1 date2 [country]\n\n"
                   "/exit\n\n ~$:");
}


