//
// Created by AriannaK97 on 12/3/20.
//

#ifndef DISEASEMONITOR_HASHTABLE_H
#define DISEASEMONITOR_HASHTABLE_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "list_lib.h"
#include "redBlackTree.h"
#include "binaryMaxHeap.h"

#define DATA_SPACE 32

#define SEARCH 1
#define PRINT 2
#define COUNT_ALL 3
#define COUNT_HOSPITALISED 4
#define COUNT_ALL_BETWEEN_DATES 5
#define COUNT_ALL_BETWEEN_DATES_WITH_VIRUS 6
#define COUNT_ALL_BETWEEN_DATES_WITH_VIRUS_AND_COUNTRY 7
#define GET_HEAP_NODES_VIRUS 8
#define GET_HEAP_NODES_COUNTRY 9
#define GET_HEAP_NODES_VIRUS_DATES 10
#define GET_HEAP_NODES_COUNTRY_DATES 11

typedef struct BucketEntry{
    char* data;
    struct rbTree* tree;
    unsigned int key;
}BucketEntry;

//Hashtable element structure
typedef struct Bucket {
    BucketEntry* entry;	// Pointer to the stored element 8
    struct Bucket* next; // Next element in case of a collision 8
    int numOfEntries;//the space the data we have already entered occupy in the bucket 4
    size_t bucketSize; //8
}Bucket;

//Hashtable structure
typedef struct HashTable{
    unsigned int capacity;	// Hashtable capacity (in terms of hashed keys)
    unsigned int e_num;	// Number of element currently stored in the hashtable
    Bucket** table;	// The table containing elements
}HashTable;

//Structure used for iterations
typedef struct HashElement{
    HashTable* ht; 	// The hashtable on which we iterate
    unsigned int index;	// Current index in the table
    Bucket* elem; 	// Current element in the list
    int counter;    //used when we count all the entries in the system
    Date* date1;    //for operations requiring date boundaries
    Date* date2;
    char* country; //for operations requiring country
    char* virus;   //for operations requiring the virus for the search
    List* heapNodes;    //nodes collected for the heap
}HashElement;


// Inititalize hashtable iterator on hashtable 'ht'
#define hashITERATOR(ht) {ht, 0, ht->table[0], 0, 0, 0, 0, 0}

unsigned long hash(unsigned long x);

HashTable* hashCreate(unsigned int);

void* hashPut(HashTable* hTable, unsigned long key, void* data, size_t bucketSize, Node* listNode);

void* hashGet(HashTable*, unsigned long);

Bucket* hashIterate(HashElement*, int operationCall);

void* hashIterateValues(HashElement* iterator, int operationCall);

bool bucketHasSpace(Bucket *bucket);

void applyOperationOnHashTable(HashTable* hTable, int operationCall);

void putInBucketData(Bucket* bucket, size_t bucketSize, char* data, HashTable* hTable, unsigned long key, Node* listNode);

int iterateBucketData(Bucket* bucket, int operationCall, HashElement* hashIterator);

void freeHashTable(HashTable* hTable);

Bucket* getBucket(size_t bucketSize, Bucket *prevBucket);
#endif //DISEASEMONITOR_HASHTABLE_H
