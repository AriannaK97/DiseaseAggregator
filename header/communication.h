//
// Created by AriannaK97 on 26/5/20.
//

#ifndef DISEASEMONITOR_CLIENT_COMMUNICATION_H
#define DISEASEMONITOR_CLIENT_COMMUNICATION_H

#include <sys/stat.h>
#include "diseaseAggregator.h"

void createNewFifoPipe(char* fileName);

int openFifoToRead(char *fileName);

int openFifoToWrite(char *fileName);

char* readFromFifoPipe(int fd_client_r, size_t bufferSize);

void writeInFifoPipe(int fd_client_w, char* message ,size_t bufferSize);

#endif //DISEASEMONITOR_CLIENT_COMMUNICATION_H
