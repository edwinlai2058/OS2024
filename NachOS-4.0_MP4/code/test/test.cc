#include "filehdr.h"
#include "pbitmap.h"
#include "sysdep.h"

void TestFileHeaderSize() {
    PersistentBitmap *freeMap = new PersistentBitmap(NumSectors);

    // 測試小檔案
    FileHeader *smallFileHeader = new FileHeader();
    smallFileHeader->Allocate(freeMap, MaxDirectSize / 2);
    printf("Small file header size: %d\n", smallFileHeader->CountHeaders());
    smallFileHeader->Deallocate(freeMap);
    delete smallFileHeader;

    // 測試中等檔案
    FileHeader *mediumFileHeader = new FileHeader();
    mediumFileHeader->Allocate(freeMap, MaxSingleIndirectSize / 2);
    printf("Medium file header size: %d\n", mediumFileHeader->CountHeaders());
    mediumFileHeader->Deallocate(freeMap);
    delete mediumFileHeader;

    // 測試大檔案
    FileHeader *largeFileHeader = new FileHeader();
    largeFileHeader->Allocate(freeMap, MaxDoubleIndirectSize / 2);
    printf("Large file header size: %d\n", largeFileHeader->CountHeaders());
    largeFileHeader->Deallocate(freeMap);
    delete largeFileHeader;

    delete freeMap;
}

int main() {
    TestFileHeaderSize();
    return 0;
}