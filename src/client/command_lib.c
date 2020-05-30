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
        //fprintf(stdout, "Total number of patients counted: %d\n~$:", iterator.counter);
    }else {
        iterator.country = country;
        while (hashIterateValues(&iterator, COUNT_ALL_BETWEEN_DATES_WITH_VIRUS_AND_COUNTRY) != NULL);
        //fprintf(stdout, "Total number of patients counted: %d\n~$:", iterator.counter);
    }
    if(iterator.counter == 0){
        fprintf(stdout, "error\n");
    }
    fprintf(stdout, "\n~$:");
}


/**
 * For the specified country and disease the application prints the age categories with the top k
 * number of diseased cases of the given disease during the period date1, date2, if that is specified.
 * Cmd Args: k country disease date1, date2
 * */
void topk_AgeRanges(CmdManager* manager, int k, char* country, char* disease , Date* date1, Date* date2, FileDiseaseStats* fileStats){
    HashElement iterator = hashITERATOR(manager->diseaseHashTable);
    iterator.country = country;
    iterator.virus = disease;
    //Heap* maxHeap = createHeap();
    if(date1 != NULL && date2 != NULL) {
        iterator.date1 = date1;
        iterator.date2 = date2;
        while (hashIterateValues(&iterator, GET_HEAP_NODES_AGE_RANGE_DATES) != NULL);
    }else {
        /*used for statistics collection where the dates are redundant*/
        while(hashIterateValues(&iterator, GET_HEAP_NODES_AGE_RANGE) != NULL);
    }
    if(iterator.AgeRangeNodes == NULL || iterator.AgeRangeNodes->head == NULL){
        //fprintf(stdout, "There are no countries with cases of %s\n~$:", disease);
        fprintf(stdout, "error\n~$:");
        //freeHeapTree(maxHeap);
    }else{
        Node* currentNode = iterator.AgeRangeNodes->head;
        AgeRangeStruct* item;
        while(currentNode != NULL){
            item = currentNode->item;
            if (item->data <= 20){
                fileStats->AgeRangeCasesArray[0] = item->dataSum;
            }else if(item->data <= 40){
                fileStats->AgeRangeCasesArray[1] = item->dataSum;
            }else if(item->data <= 60){
                fileStats->AgeRangeCasesArray[2] = item->dataSum;
            }else if(item->data <= 120){
                fileStats->AgeRangeCasesArray[3] = item->dataSum;
            }
            currentNode = currentNode->next;
        }
        if(k > 4){
            k = 4;
        }

        while (k > 0){
            if (k==0){
                fprintf(stdout, "Age range 0-20: %d\n", fileStats->AgeRangeCasesArray[k]);
            }else if(k==1){
                fprintf(stdout, "Age range 21-40: %d\n", fileStats->AgeRangeCasesArray[k]);
            }else if(k==2){
                fprintf(stdout, "Age range 41-60: %d\n", fileStats->AgeRangeCasesArray[k]);
            }else if(k==3){
                fprintf(stdout, "Age range 60+: %d\n", fileStats->AgeRangeCasesArray[k]);
            }
            k--;
        }
        fprintf(stdout, "~$:");
    }
}

/**
 * Search the record in system with the given recordID
 * Cmd Args: recordID
 * */
void searchPatientRecord(CmdManager* manager, char* recordID){
    PatientCase* patient;
    patient = getPatientFromList(manager->patientList, recordID);
    fprintf(stdout,"case number: %s | name: %s | surname: %s | virus: %s | country: %s | importDate: %d-%d-%d | "
                   "exportDate: %d-%d-%d\n", patient->recordID, patient->name, patient->surname, patient->virus,
            patient->country, patient->entryDate->day, patient->entryDate->month, patient->entryDate->year,
            patient->exitDate->day, patient->exitDate->month, patient->exitDate->year);
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
    HashElement iterator = hashITERATOR(manager->diseaseHashTable);
    int countryExists = false;
    if(country != NULL){
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
                        int patientsNum = countPatients_BetweenDates(bucket->entry[i].tree, COUNT_HOSPITALISED_BETWEEN_DATES_WITH_DISEASE, &iterator);
                        if(strlen(bucket->entry[i].data)!=0)
                            fprintf(stdout, "%s %d\n~$:", country, patientsNum);
                        //fprintf(stdout, "The number of the currently hospitalised patients for %s is: %d\n~$:", disease, patientsNum);
                        countryExists = 1;
                        break;
                    }
                }
                if(countryExists)
                    break;
                bucket = bucket->next;
            }
            if(!countryExists){
                fprintf(stdout, "%s 0\n~$:", country);
                //fprintf(stdout, "The disease %s does not exist in the system\n~$:", disease);
            }
        }else
            fprintf(stdout, "%s 0\n~$:", country);
        //fprintf(stdout, "The number of hospitalised patients for %s: 0\n~$:", disease);
    }else{
        int totalCounted = 0;
        iterator.date1 = date1;
        iterator.date2 = date2;
        iterator.virus = disease;
        for (unsigned int h = 0; h < manager->countryHashTable->capacity; h++ ){
            Bucket* bucket = manager->countryHashTable->table[h];
            if(bucket != NULL){
                while (bucket != NULL){
                    for(int i = 0; i < bucket->numOfEntries; i++){
                        int patientsNum = countPatients_BetweenDates(bucket->entry[i].tree, COUNT_HOSPITALISED_BETWEEN_DATES_WITH_DISEASE, &iterator);
                        if(strlen(bucket->entry[i].data)!=0)
                            fprintf(stdout, "%s %d\n", bucket->entry[i].data, patientsNum);
                        //fprintf(stdout, "The number of the currently hospitalised patients for %s is: %d\n~$:", bucket->entry[i].data, patientsNum);
                        totalCounted += patientsNum;
                    }
                    bucket = bucket->next;
                }
            }
        }
        fprintf(stdout, "\n~$:");
        //fprintf(stdout, "Total number of patients counted: %d\n~$:", totalCounted);
    }
}


void numPatientDischarges(CmdManager* manager, char* disease, Date* date1, Date* date2, char* country){
    HashElement iterator = hashITERATOR(manager->diseaseHashTable);
    int countryExists = false;
    if(country != NULL){
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
                        int patientsNum = countPatients_BetweenDates(bucket->entry[i].tree, COUNT_HOSPITALISED_BETWEEN_DATES_WITH_DISEASE_EXIT, &iterator);
                        if(strlen(bucket->entry[i].data)!=0)
                            fprintf(stdout, "%s %d\n~$:", country, patientsNum);
                        //fprintf(stdout, "The number of the currently hospitalised patients for %s is: %d\n~$:", disease, patientsNum);
                        countryExists = 1;
                        break;
                    }
                }
                if(countryExists)
                    break;
                bucket = bucket->next;
            }
            if(!countryExists){
                fprintf(stdout, "%s 0\n~$:", country);
                //fprintf(stdout, "The disease %s does not exist in the system\n~$:", disease);
            }
        }else
            fprintf(stdout, "%s 0\n~$:", country);
        //fprintf(stdout, "The number of hospitalised patients for %s: 0\n~$:", disease);
    }else{
        int totalCounted = 0;
        iterator.date1 = date1;
        iterator.date2 = date2;
        iterator.virus = disease;
        for (unsigned int h = 0; h < manager->countryHashTable->capacity; h++ ){
            Bucket* bucket = manager->countryHashTable->table[h];
            if(bucket != NULL){
                while (bucket != NULL){
                    for(int i = 0; i < bucket->numOfEntries; i++){
                        int patientsNum = countPatients_BetweenDates(bucket->entry[i].tree, COUNT_HOSPITALISED_BETWEEN_DATES_WITH_DISEASE_EXIT, &iterator);
                        if(strlen(bucket->entry[i].data)!=0)
                            fprintf(stdout, "%s %d\n", bucket->entry[i].data, patientsNum);
                        //fprintf(stdout, "The number of the currently hospitalised patients for %s is: %d\n~$:", bucket->entry[i].data, patientsNum);
                        totalCounted += patientsNum;
                    }
                    bucket = bucket->next;
                }
            }
        }
        fprintf(stdout, "\n~$:");
        //fprintf(stdout, "Total number of patients counted: %d\n~$:", totalCounted);
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


void commandServer(CmdManager* manager, char* line){
    char* command = NULL;
    char* simpleCommand = NULL;
    //char* line = NULL;

    //fprintf(stdout,"~$:");
    //while (getline(&line, &length, stdin) != EOF){

    simpleCommand = strtok(line, "\n");
    if(simpleCommand == NULL){
        return;
    }else if(strcmp(simpleCommand, "/help") == 0){
        helpDesc();
    } else if(strcmp(simpleCommand, "/exit") == 0){
        free(line);
        exitMonitor(manager);
    }else {

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
                topk_AgeRanges(manager, k, country, disease, date1, date2, NULL);
                free(date1);
                free(date2);

            } else if (arg3 == NULL || arg4 == NULL) {
                fprintf(stderr, "Missing date space. Please try again...\n~$:");
            }

        } else if (strcmp(command, "/searchPatientRecord") == 0) {

                char* recordID = strtok(NULL, "\n");
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
        } else {
            fprintf(stdout,"The command you have entered does not exist.\n You can see the "
                           "available commands by hitting /help.\n~$:");
        }
    }
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


