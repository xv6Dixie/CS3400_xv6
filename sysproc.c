#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int sys_fork(void) {
    return fork();
}

int sys_fork_original(void) {
    return fork_original();
}

int sys_exit(void) {
    exit();
    return 0; // not reached
}

int sys_wait(void) {
    return wait();
}

int sys_kill(void) {
    int pid;

    if(argint(0, &pid) < 0)
        return -1;
    return kill(pid);
}

int sys_getpid(void) {
    return myproc()->pid;
}

int sys_sbrk(void) {
    int addr;
    int n;

    if(argint(0, &n) < 0)
        return -1;
    addr = myproc()->sz;
    if(growproc(n) < 0)
        return -1;
    return addr;
}

int sys_sleep(void) {
    int n;
    uint ticks0;

    if(argint(0, &n) < 0)
        return -1;
    acquire(&tickslock);
    ticks0 = ticks;
    while(ticks - ticks0 < n) {
        if(myproc()->killed) {
            release(&tickslock);
            return -1;
        }
        sleep(&ticks, &tickslock);
    }
    release(&tickslock);
    return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int sys_uptime(void) {
    uint xticks;

    acquire(&tickslock);
    xticks = ticks;
    release(&tickslock);
    return xticks;
}

extern int chtickets(int, int);

int sys_getNumFreePages(void) {
    return getNumFreePages();
}

int sys_setTickets(void) {
    struct proc *p = myproc();
    int n;

    if(argint(0, &n) < 0)
        return -1;

    p->tickets = n;
    return n;
}

int sys_chtickets(void) {
    int pid, tickets;
    if(argint(0, &pid) < 0)
        return -1;
    if(argint(1, &tickets) < 0)
        return -1;

    return chtickets(pid, tickets);
}

int sys_nice(void) {
    int n;
    struct proc *proc = myproc();
    if(argint(0,&n) < 0)
        return -1;
    proc->priority = n;
    return 0;
}

int sys_getpri(void) {
    struct proc *proc = myproc();
    if (proc != 0) {
        cprintf("PID:%d, Parent's PID:%d ,priority:%d\n",proc->pid, proc->parent->pid, proc->priority);
        return 0;
    }
    return -1;
}