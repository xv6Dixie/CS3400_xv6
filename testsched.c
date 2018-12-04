#include "types.h"
#include "defs.h"
#include "stat.h"
#include "mmu.h"
#include "param.h"
#include "proc.h"

int main(void) {
    int rezz = pdump();

    exit();
}
