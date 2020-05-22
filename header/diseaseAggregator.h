//
// Created by AriannaK97 on 21/5/20.
//

#ifndef DISEASEAGGREGATOR_DISEASEAGGREGATOR_H
#define DISEASEAGGREGATOR_DISEASEAGGREGATOR_H

#include <signal.h>
#include "list_lib.h"
#include "structs.h"
#include <sys/types.h>
#include <unistd.h>
#include "../header/data_io.h"
#include <dirent.h>
#include <errno.h>
#include <stdio.h>

typedef struct Message{
    pid_t m_clientpid;
    char sm_data[200];
}Message;

typedef struct DirListItem{
    char dirName[256];
    char dirPath[256];
}DirListItem;

typedef struct FileItem{
    char* filePath;
    char* fileName;
    Date* dateFile;
}FileItem;

typedef struct FileExplorer{
    char* country;  /*current directory*/
    int failedEntries;
    int successfulEntries;
    FileItem** fileArray;
}FileExplorer;

typedef struct Aggregatormanager{
    List** directoryDistributor;
}AggregatorManager;

typedef struct AggregatorInputArguments{
    size_t bufferSize;
    int numWorkers;
    char *input_dir;
}AggregatorInputArguments;

AggregatorManager* readDirectoryFiles(AggregatorInputArguments* arguments);

int countFilesInDirectory(DIR *FD);

FileItem* createFileArray(DIR * FD, DirListItem* item, int arraySize);

AggregatorInputArguments* getAggregatorInputArgs(int argc, char** argv);

void printAggregatorManagerDirectoryDistributor(AggregatorManager* aggregatorManager, int numOfWorkers);

#endif //DISEASEAGGREGATOR_DISEASEAGGREGATOR_H
