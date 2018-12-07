//
// Created by pwatkin1 on 12/4/2018.
//

#ifndef CS3400_XV6_PSTAT_H
#define CS3400_XV6_PSTAT_H

#include "param.h"

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

struct pstat {
    int inuse[NPROC]; // whether this slot of the process table is in use (1 or 0)
    int pid[NPROC];   // PID of each process
    int priority[NPROC]; // current priority level of each process (0-3)
    enum procstate state[NPROC];  // current state (e.g., SLEEPING or RUNNABLE) of each process
    int ticks[NPROC][NPRIOR]; // number of ticks each process has accumulated at each of 3 priorities
};

#endif //CS3400_XV6_PSTAT_H
