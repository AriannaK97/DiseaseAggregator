//
// Created by linuxuser on 22/5/20.
//

#include "../../header/server.h"
#include <stdio.h>

bool make_fifo_name(int workerNum, char *name, size_t name_max){
    sprintf(name, "worker%d", workerNum);
    return true;
}