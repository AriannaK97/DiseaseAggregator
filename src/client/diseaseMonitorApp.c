//
// Created by AriannaK97 on 22/5/20.
//

#include <stdio.h>
#include "../../header/data_io.h"
#include "../../header/command_lib.h"

int main(int argc, char** argv) {


/*****************************************************************************
 *                       Handling command line arguments                     *
 *****************************************************************************/
    MonitorInputArguments* arguments = getMonitorInputArgs(argc, argv);
/*****************************************************************************
 *                       Handling input files                                *
 *****************************************************************************/
    FILE *patientRecordsFile;
    //patientRecordsFile = openFile(arguments->inputFile);
    CmdManager* cmdManager;

    fprintf(stdout, "I am the client\n");
/*    cmdManager = read_input_file(patientRecordsFile, getMaxFromFile(patientRecordsFile, LINE_LENGTH),
                                 arguments->diseaseHashtableNumOfEntries,arguments->countryHashTableNumOfEntries, arguments->bucketSize);*/

    free(arguments);
    fclose(patientRecordsFile);

    /**
     * Uncomment the line below to see all the inserted patients in the list
     * */

    //printList(cmdManager->patientList);

    /**
     * Uncomment the two lines below to see the hashtable contents
     * */
    //applyOperationOnHashTable(cmdManager->diseaseHashTable, PRINT);
    //applyOperationOnHashTable(cmdManager->countryHashTable, PRINT);

    commandServer(cmdManager);

    return 0;
}