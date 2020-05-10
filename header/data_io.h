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

#define BUCKET_SIZE 120              /*bucket size for just one entry - minimum is 52*/
#define DISEASE_HT_Entries_NUM 10   /*size of diseaseHashTable*/
#define COUNTRY_HT_Entries_NUM 10   /*size of countryHashTable*/
#define BUFFER_SIZE 52              /*minimum buffer size for reading over pipes*/

enum defAttribute{
    LINE_LENGTH,
    LINE_BUFFER_SIZE,
};

CmdManager* readDirectoryFiles_And_PopulateAggregator(InputArguments* arguments, CmdManager* cmdManager);

PatientCase* getPatient(char* buffer, FileExplorer* fileExplorer);

FILE* openFile(char *inputFile);

InputArguments* getInputArgs(int argc, char** argv);

int getMaxFromFile(FILE* patientRecordsFile, int returnVal);

bool writeEntry(char* buffer, List* patientList, HashTable* diseaseHashTable, HashTable* countryHashTable, int bucketSize);

CmdManager* read_input_file(FILE* patientRecordsFile, size_t maxStrLength, CmdManager* cmdManager, FileExplorer* fileExplorer);

bool dateInputValidation(Date* entryDate, Date* exitDate);

bool setDate(PatientCase *patient, char *buffer);

CmdManager* initializeStructures(int diseaseHashtableNumOfEntries, int countryHashTableNumOfEntries, size_t bucketSize);

void deallockFileExplorer(FileExplorer *fileExplorer);
#endif //DISEASEMONITOR_DATA_IO_H
