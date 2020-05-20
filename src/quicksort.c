#include "quicksort.h"
#include "relation.h"
#include "auxLib.h"
#include <string.h>

/**
 *QuickSort recursive
 **/
void quickSort(Tuple* bucket, int low, int high, int recordField){

    int pivot;          //partitioning index

    if (low < high) {
        pivot = partition(bucket, low, high, recordField);
        quickSort(bucket, low, pivot - 1, recordField);
        quickSort(bucket, pivot + 1, high, recordField);
    }

}

/**
 *Getting the record tuple of a certain position in the bucket
 **/
Tuple getRecordAtPosition(Tuple* bucket, int position){
    Tuple tuple;
    memcpy(&tuple, &bucket[position], sizeof(Tuple));
    return tuple;
}


/**
 *Partition rearranges the records
 **/
int partition(Tuple* bucket, int low, int high, int recordField){

    int i = low-1;
    Tuple pivotRecord;
    Tuple cmpRecordB;

    pivotRecord = getRecordAtPosition(bucket,high);
    for(int j = low; j <= high-1; j++){
        cmpRecordB = getRecordAtPosition(bucket,j);
        if(compare(cmpRecordB, pivotRecord, recordField)==LessThanEqual){
            i++;
            swap(bucket,i,j);
        }
    }
    swap(bucket,i+1,high);
    return (i+1);
}

/**
 * Swapping the records between their positions in blockBuffer
 **/
void swap(Tuple* bucket, int positionA, int positionB){
    Tuple recordA;
    Tuple recordB;
    Tuple tempRecord;

    recordA = getRecordAtPosition(bucket, positionA);
    recordB = getRecordAtPosition(bucket, positionB);
    memcpy(&tempRecord, &recordA, sizeof(Tuple));
    memcpy(&bucket[positionB], &recordA, sizeof(Tuple));
    memcpy(&bucket[positionA], &recordB, sizeof(Tuple));
}

/**
 *Compare: comparing tuple values either of field type KEY or PAYLOAD
 **/
ErrorCode compare(Tuple recordA, Tuple recordB, int recordField){
    if(recordField == KEY){
        if(recordA.key <= recordB.key)
            return LessThanEqual;
        else
            return NotEqual;
    }else if (recordField == PAYLOAD){
        if(recordA.payload <= recordB.payload)
            return LessThanEqual;
        else
            return NotEqual;
    }else
        return RecordFieldTypeError;
}