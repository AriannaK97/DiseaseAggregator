//
// Created by AriannaK97 on 12/3/20.
//

#ifndef DISEASEMONITOR_COMMAND_LIB_H
#define DISEASEMONITOR_COMMAND_LIB_H

#include "diseaseAggregator.h"


typedef struct CmdManager CmdManager;
typedef struct Date Date;

void listCountries(AggregatorServerManager* aggregatorServerManager);

void diseaseFrequency(CmdManager* manager, char* virusName, Date* date1, Date* date2, char* country);

void topk_AgeRanges(CmdManager* manager, int k, char* country, char* disease , Date* date1, Date* date2, FileDiseaseStats* fileStats);

void searchPatientRecord(CmdManager* manager, char* recordID);

void numPatientAdmissions(CmdManager* manager, char* disease, Date* date1, Date* date2, char* country);

void numPatientDischarges(CmdManager* manager, char* disease, Date* date1, Date* date2, char* country);

void exitMonitor(CmdManager* manager);

void commandServer(CmdManager* manager, char* line);

void helpDesc();

#endif //DISEASEMONITOR_COMMAND_LIB_H
