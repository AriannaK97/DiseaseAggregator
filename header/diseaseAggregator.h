//
// Created by AriannaK97 on 21/5/20.
//

#ifndef DISEASEAGGREGATOR_DISEASEAGGREGATOR_H
#define DISEASEAGGREGATOR_DISEASEAGGREGATOR_H

#define  _POSIX_C_SOURCE 200809L

#include <signal.h>
#include "list_lib.h"
#include "structs.h"
#include <stdlib.h>
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

typedef struct FileDiseaseStats{
    char* disease;
    int* AgeRangeCasesArray;
}FileDiseaseStats;

typedef struct FileItem{
    char* filePath;
    char* fileName;
    Date* dateFile;
    FileDiseaseStats** fileDiseaseStats;
    int numOfDiseases;
}FileItem;

typedef struct FileExplorer{
    char* country;  /*current directory*/
    int failedEntries;
    int successfulEntries;
    FileItem* fileItemsArray;
    int fileArraySize;
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
    int fd_client_w;
    int fd_client_r;
}WorkerInfo;

typedef struct AggregatorServerManager{
    List** directoryDistributor;
    int numOfWorkers;
    WorkerInfo* workersArray;
    size_t bufferSize;
    char *input_dir;
}AggregatorServerManager;

void freeAggregatorManager(AggregatorServerManager *aggregatorManager);

void freeAggregatorInputArguments(AggregatorInputArguments *aggregatorInputArguments);

bool make_fifo_name_server_client(pid_t workerNum, char *name);

bool make_fifo_name_client_server(pid_t workerNum, char *name);

AggregatorServerManager* readDirectoryFiles(AggregatorInputArguments* arguments);

int countFilesInDirectory(DIR *FD);

FileItem* createFileArray(DIR * FD, DirListItem* item, int arraySize, int bufferSize);

AggregatorInputArguments* getAggregatorInputArgs(int argc, char** argv);

void printAggregatorManagerDirectoryDistributor(AggregatorServerManager* aggregatorManager, int numOfWorkers);

void DiseaseAggregatorServerManager(AggregatorServerManager* aggregatorServerManager);

bool sendStatistics(CmdManager* cmdManager);

bool receiveStats(AggregatorServerManager* aggregatorServerManager, int workerId);

void exitAggregator(AggregatorServerManager* aggregatorServerManager, char* command);

void deallockWorkerInfo(WorkerInfo* workerInfo);

void deallockFileItem(FileItem* fileItem);

void nodeDirListItemDeallock(DirListItem* dirListItem);

#endif //DISEASEAGGREGATOR_DISEASEAGGREGATOR_H
