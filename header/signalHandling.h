//
// Created by AriannaK97 on 2/6/20.
//

#ifndef DISEASEMONITOR_CLIENT_SIGNALHANDLING_H
#define DISEASEMONITOR_CLIENT_SIGNALHANDLING_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "diseaseAggregator.h"

pid_t globPid;
List* globDirList;
WorkerLog* globWorkerLog;

void sigintHandler(int signal);

#endif //DISEASEMONITOR_CLIENT_SIGNALHANDLING_H
