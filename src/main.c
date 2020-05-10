#include <stdio.h>
#include "../header/data_io.h"
#include "../header/command_lib.h"

int main(int argc, char** argv) {


/*****************************************************************************
 *                       Handling command line arguments                     *
 *****************************************************************************/
    InputArguments* arguments = getInputArgs(argc, argv);
/*****************************************************************************
 *                       Handling input files                                *
 *****************************************************************************/
    //FILE *patientRecordsFile;
    CmdManager* cmdManager;
    cmdManager = initializeStructures(DISEASE_HT_Entries_NUM, COUNTRY_HT_Entries_NUM, BUCKET_SIZE);
    cmdManager = readDirectoryFiles_And_PopulateAggregator(arguments, cmdManager);

    free(arguments->input_dir);
    free(arguments);

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