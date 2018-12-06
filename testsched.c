#include "types.h"
#include "user.h"
//#include "defs.h"
#include "stat.h"
#include "mmu.h"
#include "param.h"
#include "proc.h"
#include "pstat.h"

// Runs multiple different process types and averages the results
void test1() {
    int i, n, k, retime, rutime, stime, pid;
    int j = 0;
    int sums[3][3];

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            sums[i][j] = 0;
        }
    }

    n = 6;

    for (i = 0; i < n; i++) {
        j = i % 3;

        pid = fork();
        if (pid == 0) {//child
            j = (getpid() - 4) % 3; // ensures independence from the first son's pid when gathering the results in the second part of the program
            switch(j) {
            case 0:     //CPU‐bound process (CPU):
                for (double z = 0; z < 1000000.0; z+= 0.1) {
                    double x =  x + 3.14 * 89.64 / 2.5;       // useless calculations to consume CPU time
                }
                break;
            case 1:     //short tasks based CPU‐bound process (S‐CPU):
                for (k = 0; k < 100; k++) {
                    for (j = 0; j < 1000000; j++) {
                        // Was the yield supposed to be here???
                    }
                    yield();
                }
                break;
            case 2:    // simulate I/O bound process (IO)
                for(k = 0; k < 100; k++) {
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
        case 0:     // Large job processes
            printf(1, "Large Job, pid: %d, ready: %d, running: %d, sleeping: %d, turnaround: %d\n", pid, retime, rutime, stime, retime + rutime + stime);
            sums[0][0] += retime;
            sums[0][1] += rutime;
            sums[0][2] += stime;
            break;
        case 1:     // short tasks
            printf(1, "Short Job, pid: %d, ready: %d, running: %d, sleeping: %d, turnaround: %d\n", pid, retime, rutime, stime, retime + rutime + stime);
            sums[1][0] += retime;
            sums[1][1] += rutime;
            sums[1][2] += stime;
            break;
        case 2:     // simulating I/O bound processes
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

void test2(void) {
    struct pstat ps;
    int pid = fork();
    if (pid > 0) {
        nice(2);
        sleep(1);
        getpri();
        wait();
    } else {
        nice(0);
        sleep(1);
        getpri();
    }
    pdump();

    print_proc_info(&ps, 1);
    //exit();
}

int main(int argc, char *argv[]) {

    test1();
    test2();

    exit();
}
