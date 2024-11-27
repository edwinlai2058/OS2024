/* add.c
 *	Simple program to test whether the systemcall interface works.
 *
 *	Just do a add syscall that adds two values and returns the result.
 *
 */

#include "syscall.h"
int main() {
    int result;

    result = Add(42, 23);
    PrintInt(result);
    // SysHalt() is in the end of MSG()
    MSG("add~~~~");
    /* not reached */
}
