//
// Created by AriannaK97 on 15/3/20.
//

#ifndef DISEASEMONITOR_STRUCTS_H
#define DISEASEMONITOR_STRUCTS_H

typedef struct BucketEntry BucketEntry;
typedef struct Bucket Bucket;
typedef struct HashTable HashTable;
typedef struct HashElement HashElement;

typedef struct Date{
    int day;
    int month;
    int year;
}Date;

typedef struct PatientCase{
    char* caseNum;
    char* name;
    char* surname;
    char* virus;
    char* country;
    struct Date* entryDate;
    struct Date* exitDate;
}PatientCase;

typedef struct Node{
    void* item;
    struct Node* next;
}Node;

typedef struct List{
    struct Node* head;
    struct Node* tail;
}List;

typedef struct CmdManager{
    struct List* patientList;
    struct HashTable* diseaseHashTable;
    struct HashTable* countryHashTable;
    size_t bucketSize;
}CmdManager;


typedef struct InputArguments{
    size_t bucketSize;
    int diseaseHashtableNumOfEntries;
    int countryHashTableNumOfEntries;
    char *inputFile;
}InputArguments;

#endif //DISEASEMONITOR_STRUCTS_H
