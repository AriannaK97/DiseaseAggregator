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
    char* recordID;
    char* type;
    char* name;
    char* surname;
    char* virus;
    char* country;
    int age;
    struct Date* entryDate;
    struct Date* exitDate;
}PatientCase;

typedef struct FileExplorer{
    char* country;  /*current directory*/
    char* date;     /*current filename*/
    int failedEntries;
    int successfulEntries;
    char** fileNameArray;
}FileExplorer;

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
    FileExplorer* fileExplorer;
    size_t bucketSize;
}CmdManager;


typedef struct InputArguments{
    size_t bufferSize;
    int numWorkers;
    char *input_dir;
}InputArguments;

#endif //DISEASEMONITOR_STRUCTS_H
