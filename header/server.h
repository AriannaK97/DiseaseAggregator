//
// Created by linuxuser on 22/5/20.
//

#ifndef DISEASEAGGREGATOR_SERVER_H
#define DISEASEAGGREGATOR_SERVER_H
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>

#define PERM_FILE 0666
#define SERVER_FIFO_NAME "aggregator_server"
#define READ 0
#define WRITE 1

typedef struct WorkerInfo{
    pid_t workerPid;
    char *serverFileName;
}WorkerInfo;

typedef struct ServerInfo{
    int numOfWorkers;
    WorkerInfo *workersArray;
}ServerInfo;

bool make_fifo_name(int workerNum, char *name, size_t name_max);

#endif //DISEASEAGGREGATOR_SERVER_H
