#include "stat.h"
#include "user.h"

int main() {
    struct ptable pt;
    struct proc *p;

    for(p = pt.proc; p < &pt.proc[NPROC]; p++) {
        if(p->parent == curproc) {
            p->parent = initproc;
            if(p->state == ZOMBIE)
                wakeup1(initproc);
        }
    }
}
