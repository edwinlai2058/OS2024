#include "syscall.h"

int main() {
    int fid1 = Open("file1.test");
    int fid2 = Open("file1.test");
    int fid3 = Open("add.c");
    int fid4 = Open("createFile.c");
    int fid5 = Open("add.c");
    int fid6 = Open("createFile.c");


    PrintInt(fid1);
    PrintInt(fid2);
    PrintInt(fid3);
    PrintInt(fid4);
    PrintInt(fid5);
    PrintInt(fid6);

    Halt();
}