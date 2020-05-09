//
// Created by AriannaK97 on 9/3/20.
//

#ifndef DISEASEMONITOR_DATA_IO_H
#define DISEASEMONITOR_DATA_IO_H


#include <stdlib.h>
#include <string.h>
#include "list_lib.h"
#include "hashTable.h"
#include "structs.h"

#define BUCKET_SIZE 52              /*minimum bucket size for just one entry*/

enum defAttribute{
    LINE_LENGTH,
    BUFFER_SIZE,
};

PatientCase* getPatient(char* buffer);

FILE* openFile(char *inputFile);

InputArguments* getInputArgs(int argc, char** argv);

int getMaxFromFile(FILE* patientRecordsFile, int returnVal);

bool writeEntry(char* buffer, List* patientList, HashTable* diseaseHashTable, HashTable* countryHashTable, int bucketSize);

CmdManager* read_input_file(FILE* patientRecordsFile, size_t maxStrLength, int diseaseHashtableNumOfEntries,
                            int countryHashTableNumOfEntries, size_t bucketSize);

bool dateInputValidation(Date* entryDate, Date* exitDate);

#endif //DISEASEMONITOR_DATA_IO_H
