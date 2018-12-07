#include "types.h"
#include "user.h"
//#include "defs.h"
#include "stat.h"
#include "mmu.h"
#include "param.h"
#include "proc.h"
#include "pstat.h"

#define MAX_PROC 5000

//#define VERBOSE

struct ptime {
    int pid[MAX_PROC];
    int ticks_start[MAX_PROC];
    int ticks_end[MAX_PROC];
    int nprocs;
};

static struct ptime ptime_rcd;

void ptime_rcd_init(struct ptime* prcd) {
    prcd->nprocs = 0;
}

void ptime_rcd_add_start(struct ptime* prcd, int pid, int start_time) {
    prcd->pid[prcd->nprocs] = pid;
    prcd->ticks_start[prcd->nprocs] = start_time;
    prcd->nprocs++;
}

void ptime_rcd_add_end(struct ptime* prcd, int pid, int end_time) {
    int i = 0;
    for (i = 0; i < prcd->nprocs; i++) {
        if (prcd->pid[i] == pid) {
            prcd->ticks_end[i] = end_time;
            return;
        }
    }
    printf(2, "ptime_rcd_add_end failed to find the pid\n");
}

void ptime_rcd_print(struct ptime* prcd) {
    int avg = 0;
    int turnaround = 0;
    int i = 0;
    printf(1, "process summary\n");
    for (i = 0; i < prcd->nprocs; i++) {
        turnaround = prcd->ticks_end[i] - prcd->ticks_start[i];
        avg += turnaround;
        printf(1, "pid = %d, start = %d, end = %d, turnaround = %d\n",
               prcd->pid[i], prcd->ticks_start[i], prcd->ticks_end[i], turnaround);
    }
    printf(1, "average turnaround time = %d\n", avg / prcd->nprocs);
}

void empty_loop(int n) {
    while (0 < n--) {
        (void)(n * n * n);
    }
}

void periodic_sleep(int period, int n) {
    while (0 < n--) {
        sleep(period);
    }
}

void read_then_write(char* path) {   // in fact only read
    char buf[512];
    int n;
    int fd = open(path, 0);
    if (fd < 0) {
        printf(2, "cat: cannot open %s\n", path);
        exit();
    }

    while((n = read(fd, buf, sizeof(buf))) > 0);
    // write(1, buf, n);
    if(n < 0){
        printf(2, "cat: read error\n");
        exit();
    }
}

int create_new_loop_proc(int n) {
    int pid = fork();
    if (pid < 0) {
        printf(2, "scheduler_benchmark : create_new_loop_proc failed.\n");
    } else if (pid == 0) {
        empty_loop(n);
        exit();
    }
    return pid;
}

int create_new_io_proc(char *path) {
    int pid = fork();
    if (pid < 0) {
        printf(2, "scheduler_benchmark : create_new_io_proc failed.\n");
    } else if (pid == 0) {
        read_then_write(path);
        exit();
    }
    return pid;
}

int create_new_sleep_proc(int period, int n) {
    int pid = fork();
    if (pid < 0) {
        printf(2, "scheduler_benchmark : create_new_sleep_proc failed.\n");
    } else if (pid == 0) {
        periodic_sleep(period, n);
        exit();
    }
    return pid;
}

// n concurrent processes
void benchmark1(int nprocs, int nloops, int period, int repeat) {
    printf(1, "------------------------------------------------------------------------\n");
    printf(1, "Concurrent Proc Test: num processes = %d, num loops = %d\n", nprocs, nloops);
    ptime_rcd_init(&ptime_rcd);
    struct pstat ps;
    int pi = 0;
    int pid = 0;
    int ticks = 0;
    int failed_procs = 0;
    for (pi = 0; pi < nprocs; pi++) {
        pid = create_new_loop_proc(nloops);
        if (pid < 0) {
            failed_procs++;
        } else {
            ticks = uptime();
            ptime_rcd_add_start(&ptime_rcd, pid, ticks);
#ifdef VERBOSE
            printf(1, "create pid = %d at %d\n", pid, ticks);
#endif
        }
    }

#ifdef VERBOSE
    int t = 0;
  // print the process table every period ticks
  for (t = 0; t < repeat; t++) {
//    if (getpinfo(&ps) < 0) {
//      printf(2, "getpinfo: cannot get process information");
//    }
    getpinfo(&ps);
    print_proc_info(&ps, 1);
    sleep(period);
  }
#endif

    for (pi = 0; pi < nprocs - failed_procs; pi++) {
        pid = wait();
        ticks = uptime();
        if(pid < 0) {
            printf(1, "wait stopped early\n");
            continue;
        }
        ptime_rcd_add_end(&ptime_rcd, pid, ticks);
    }
    printf(2, "all children terminated\n");
    getpinfo(&ps);
    print_proc_info(&ps, 1);
    ptime_rcd_print(&ptime_rcd);
}

// some short processes arrives, preempting long processes
void benchmark2(int longprocs, int longloops, int shortprocs, int shortloops,
           int period, int repeat) {
    printf(1, "------------------------------------------------------------------------\n");
    printf(1, "Long/Short Proc Test: num processes = %d longs, %d shorts, num loops = %d for long, %d for short\n",
           longprocs, shortprocs, longloops, shortloops);
    ptime_rcd_init(&ptime_rcd);
    struct pstat ps;
    int pi = 0;
    int pid = 0;
    int ticks = 0;
    int failed_procs = 0;
    // create long loops
    for (pi = 0; pi < longprocs; pi++) {
        pid = create_new_loop_proc(longloops);
        if (pid < 0) {
            failed_procs++;
        } else {
            ticks = uptime();
            ptime_rcd_add_start(&ptime_rcd, pid, ticks);
#ifdef VERBOSE
            printf(1, "create pid = %d at %d\n", pid, ticks);
#endif
        }
    }
#ifdef VERBOSE
    int t = 0;
  // print the process table every period ticks
  for (t = 0; t < repeat; t++) {
//    if (getpinfo(&ps) < 0) {
//      printf(2, "getpinfo: cannot get process information");
//    }
    getpinfo(&ps);
    print_proc_info(&ps, 1);
    sleep(period);
  }
#else
    sleep(period * repeat);
#endif
    // create short loops
    for (pi = 0; pi < shortprocs; pi++) {
        pid = create_new_loop_proc(shortloops);
        if (pid < 0) {
            failed_procs++;
        } else {
            ticks = uptime();
            ptime_rcd_add_start(&ptime_rcd, pid, ticks);
#ifdef VERBOSE
            printf(1, "create pid = %d at %d\n", pid, ticks);
#endif
        }
    }
#ifdef VERBOSE
    for (t = 0; t < repeat; t++) {
//    if (getpinfo(&ps) < 0) {
//      printf(2, "getpinfo: cannot get process information");
//    }
    getpinfo(&ps);
    print_proc_info(&ps, 1);
    sleep(period);
  }
#endif
    for (pi = 0; pi < longprocs + shortprocs - failed_procs; pi++) {
        pid = wait();
        ticks = uptime();
        if(pid < 0) {
            printf(1, "wait stopped early\n");
            continue;
        }
        ptime_rcd_add_end(&ptime_rcd, pid, ticks);
    }
    printf(2, "all children terminated\n");
//    if (getpinfo(&ps) < 0) {
//        printf(2, "getpinfo: cannot get process information");
//    }
    getpinfo(&ps);
    print_proc_info(&ps, 1);
    ptime_rcd_print(&ptime_rcd);
}

// CPU intensity + I/O intensity + periodically sleeping
void benchmark3(int nprocs, int nloops, int t_sleep, int repeat, char* path) {
    printf(1, "------------------------------------------------------------------------\n");
    printf(1, "I/O Sim Test: num processes = %d, num repeats\n", nprocs, repeat);
    ptime_rcd_init(&ptime_rcd);
    struct pstat ps;
    int pi = 0;
    int pid = 0;
    int ticks = 0;
    for (pi = 0; pi < nprocs; pi++) {
        pid = create_new_loop_proc(nloops);
        ticks = uptime();
        ptime_rcd_add_start(&ptime_rcd, pid, ticks);
#ifdef VERBOSE
        printf(1, "create pid = %d at %d\n", pid, ticks);
#endif
        pid = create_new_sleep_proc(t_sleep, repeat);
        ticks = uptime();
        ptime_rcd_add_start(&ptime_rcd, pid, ticks);
#ifdef VERBOSE
        printf(1, "create pid = %d at %d\n", pid, ticks);
#endif
        pid = create_new_io_proc(path);
        ticks = uptime();
        ptime_rcd_add_start(&ptime_rcd, pid, ticks);
#ifdef VERBOSE
        printf(1, "create pid = %d at %d\n", pid, ticks);
#endif
    }

#ifdef VERBOSE
    int t = 0;
  // print the process table every period ticks
  for (t = 0; t < repeat; t++) {
//    if (getpinfo(&ps) < 0) {
//      printf(2, "getpinfo: cannot get process information");
//    }
    getpinfo(&ps);
    print_proc_info(&ps, 1);
    sleep(t_sleep);
  }
#endif

    for (pi = 0; pi < 3 * nprocs; pi++) {
        pid = wait();
        ticks = uptime();
        if(pid < 0) {
            printf(1, "wait stopped early\n");
            exit();
        }
        ptime_rcd_add_end(&ptime_rcd, pid, ticks);
    }
    printf(2, "all children terminated\n");
//    if (getpinfo(&ps) < 0) {
//        printf(2, "getpinfo: cannot get process information");
//    }
    getpinfo(&ps);
    print_proc_info(&ps, 1);
    ptime_rcd_print(&ptime_rcd);

}

void runBM() {
    benchmark1(5, 100000000, 80, 5);
    benchmark1(61, 100000000, 200, 5);
    benchmark2(10, 100000000, 10, 1000, 80, 2);
    benchmark3(3, 100000000, 10, 10, "README");
}

// Runs multiple different process types and averages the results
void test1() {
    int i;
    int n;
    int j = 0;
    int k;
    int retime;
    int rutime;
    int stime;
    int sums[3][3];
    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
            sums[i][j] = 0;
    n = 6;
    int pid;
    for (i = 0; i < n; i++) {
        j = i % 3;
        pid = fork();
        if (pid == 0) {//child
            j = (getpid() - 4) % 3; // ensures independence from the first son's pid when gathering the results in the second part of the program
            switch(j) {
                case 0: //CPU‐bound process (CPU):
                    for (double z = 0; z < 1000000.0; z+= 0.1){
                        double x =  x + 3.14 * 89.64 / 2.5;   // useless calculations to consume CPU time
                    }
                    break;
                case 1: //short tasks based CPU‐bound process (S‐CPU):
                    for (k = 0; k < 100; k++){
                        for (j = 0; j < 1000000; j++){}
                        yield();
                    }
                    break;
                case 2:// simulate I/O bound process (IO)
                    for(k = 0; k < 100; k++){
                        sleep(1);
                    }
                    break;
            }
            exit(); // children exit here
        }
        continue; // father continues to spawn the next child
    }
    for (i = 0; i < n; i++) {
        pid = testwait(&retime, &rutime, &stime);

        int res = (pid - 4) % 3; // correlates to j in the dispatching loop
        switch(res) {
            case 0: // Large job processes
                printf(1, "Large Job, pid: %d, ready: %d, running: %d, sleeping: %d, turnaround: %d\n", pid, retime, rutime, stime, retime + rutime + stime);
                sums[0][0] += retime;
                sums[0][1] += rutime;
                sums[0][2] += stime;
                break;
            case 1: // short tasks
                printf(1, "Short Job, pid: %d, ready: %d, running: %d, sleeping: %d, turnaround: %d\n", pid, retime, rutime, stime, retime + rutime + stime);
                sums[1][0] += retime;
                sums[1][1] += rutime;
                sums[1][2] += stime;
                break;
            case 2: // simulating I/O bound processes
                printf(1, "Simulated I/O , pid: %d, ready: %d, running: %d, sleeping: %d, turnaround: %d\n", pid, retime, rutime, stime, retime + rutime + stime);
                sums[2][0] += retime;
                sums[2][1] += rutime;
                sums[2][2] += stime;
                break;
        }
    }
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            sums[i][j] /= n;
        }
    }
    printf(1, "\n\nLarge Jobs:\n"
              "Average ready time: %d\n"
              "Average running time: %d\n"
              "Average sleeping time: %d\n"
              "Average turnaround time: %d\n\n\n",
            sums[0][0], sums[0][1], sums[0][2], sums[0][0] + sums[0][1] + sums[0][2]);
    printf(1, "Small Jobs:\n"
              "Average ready time: %d\n"
              "Average running time: %d\n"
              "Average sleeping time: %d\n"
              "Average turnaround time: %d\n\n\n",
            sums[1][0], sums[1][1], sums[1][2], sums[1][0] + sums[1][1] + sums[1][2]);
    printf(1, "Simulated I/O:\n"
              "Average ready time: %d\n"
              "Average running time: %d\n"
              "Average sleeping time: %d\n"
              "Average turnaround time: %d\n\n\n",
            sums[2][0], sums[2][1], sums[2][2], sums[2][0] + sums[2][1] + sums[2][2]);
    //exit();
}


int main(int argc, char *argv[]) {
//    int rezz = pdump();
    test1();
//    test2();
    runBM();
    exit();

}
