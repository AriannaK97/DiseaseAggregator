#ifndef INFORMATIONSYSTEMS_QUICKSORT_H
#define INFORMATIONSYSTEMS_QUICKSORT_H

#include <stdbool.h>
#include "relation.h"
#include "auxLib.h"

void quickSort(Tuple*, int, int, int);

int partition(Tuple*, int, int, int);

void swap(Tuple*, int, int);

Tuple getRecordAtPosition(Tuple*, int);

ErrorCode compare(Tuple, Tuple, int);
#endif //INFORMATIONSYSTEMS_QUICKSORT_H
