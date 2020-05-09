//
// Created by AriannaK97 on 12/3/20.
//

#ifndef DISEASEMONITOR_COMMAND_LIB_H
#define DISEASEMONITOR_COMMAND_LIB_H

typedef struct CmdManager CmdManager;
typedef struct Date Date;

void globalDiseaseStats(CmdManager* manager, Date* date1, Date* date2);

void diseaseFrequency(CmdManager* manager, char* virusName, Date* date1, Date* date2, char* country);

void topk_Diseases(CmdManager* manager, int k, char* country, Date* date1, Date* date2);

void topk_Countries(CmdManager* manager, int k, char* disease, Date* date1, Date* date2);

void insertPatientRecord(CmdManager* manager, char* buffer);

void recordPatientExit(CmdManager* manager, char* args);

void numCurrentPatients(CmdManager* manager, char* disease);

void exitMonitor(CmdManager* manager);

void commandServer(CmdManager* manager);

void helpDesc();

#endif //DISEASEMONITOR_COMMAND_LIB_H
