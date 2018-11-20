#include "types.h"
#include "stat.h"
#include "user.h"

//TODO: would love to make scheduler change dynamic

int main(int argc, char *argv[]) {
    char *policy;

    if(argc <= 1) {
        printf(2, "usage: DEFAULT LOTTERY\n");
        exit();
    }


    policy = argv[1];
    printf(2, policy);

    exit();
}