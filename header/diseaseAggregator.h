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

#define PERM_FILE 0666
#define SERVER_FIFO_NAME "aggregator_server"
#define READ 0
#define WRITE 1

typedef struct Message{
    pid_t m_clientpid;
    char sm_data[200];
}Message;

typedef struct DirListItem{
    char *dirName;
    char *dirPath;
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
    FileItem* fileArray;
}FileExplorer;

typedef struct AggregatorInputArguments{
    size_t bufferSize;
    int numWorkers;
    char *input_dir;
}AggregatorInputArguments;

typedef struct WorkerInfo{
    pid_t workerPid;
    char *serverFileName;
    char *workerFileName;
}WorkerInfo;

typedef struct AggregatorServerManager{
    List** directoryDistributor;
    int numOfWorkers;
    WorkerInfo *workersArray;
}AggregatorServerManager;

void freeAggregatorManager(AggregatorServerManager *aggregatorManager);

void freeAggregatorInputArguments(AggregatorInputArguments *aggregatorInputArguments);

void freeWorkerInfo(WorkerInfo workerInfo);

bool make_fifo_name_server_client(pid_t workerNum, char *name);

bool make_fifo_name_client_server(pid_t workerNum, char *name);

AggregatorServerManager* readDirectoryFiles(AggregatorInputArguments* arguments);

int countFilesInDirectory(DIR *FD);

FileItem* createFileArray(DIR * FD, DirListItem* item, int arraySize);

AggregatorInputArguments* getAggregatorInputArgs(int argc, char** argv);

void printAggregatorManagerDirectoryDistributor(AggregatorServerManager* aggregatorManager, int numOfWorkers);

void nodeDirListItemDeallock(DirListItem* dirListItem);

void DiseaseAggregatorServerManager(AggregatorServerManager* aggregatorServerManager);

#endif //DISEASEAGGREGATOR_DISEASEAGGREGATOR_H
