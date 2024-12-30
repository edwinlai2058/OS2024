// filehdr.cc
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector,
//
//      Unlike in a real system, we do not keep track of file permissions,
//	ownership, last modification date, etc., in the file header.
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "filehdr.h"
#include "debug.h"
#include "synchdisk.h"
#include "main.h"

//----------------------------------------------------------------------
// MP4 mod tag
// FileHeader::FileHeader
//	There is no need to initialize a fileheader,
//	since all the information should be initialized by Allocate or FetchFrom.
//	The purpose of this function is to keep valgrind happy.
//----------------------------------------------------------------------
FileHeader::FileHeader()
{
	numBytes = -1;
	numSectors = -1;
	memset(dataSectors, -1, sizeof(dataSectors));
}

//----------------------------------------------------------------------
// MP4 mod tag
// FileHeader::~FileHeader
//	Currently, there is not need to do anything in destructor function.
//	However, if you decide to add some "in-core" data in header
//	Always remember to deallocate their space or you will leak memory
//----------------------------------------------------------------------
FileHeader::~FileHeader()
{
	// nothing to do now
}

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the size of the new file
//----------------------------------------------------------------------

bool FileHeader::Allocate(PersistentBitmap *freeMap, int fileSize)
{
	numBytes = fileSize;
	// MP4
	if(fileSize <= MaxDirectSize) {
		// Direct allocation
		level = 0;
		numSectors = divRoundUp(fileSize, SectorSize);
		if (freeMap->NumClear() < numSectors)
			return FALSE; // not enough space
		
		for (int i = 0; i < numSectors; i++) {
			dataSectors[i] = freeMap->FindAndSet();
			// since we checked that there was enough free space,
			// we expect this to succeed
			ASSERT(dataSectors[i] >= 0);
		}
	}
	else if(fileSize <= MaxSingleIndirectSize) {
		// Single indirect allocation
		level = 1;
		numSectors = divRoundUp(fileSize, MaxDirectSize);
		if(freeMap->NumClear() < numSectors)
			return FALSE; // not enough space
		
		int remainSize = fileSize;
		for(int i = 0; i < numSectors; i++) {
			dataSectors[i] = freeMap->FindAndSet();
			ASSERT(dataSectors[i] >= 0);

			FileHeader *nextHdr = new FileHeader;
			int nextLevelSize = (remainSize > MaxDirectSize) ? MaxDirectSize : remainSize;
			remainSize -= nextLevelSize;

			if(!nextHdr->Allocate(freeMap, nextLevelSize))
				return FALSE;
			else
				nextHdr->WriteBack(dataSectors[i]);
			
			delete nextHdr;
			if(remainSize <= 0)	 break;
		}
	}
	else if(fileSize <= MaxDoubleIndirectSize) {
		// Double indirect allocation
		level = 2;
		numSectors = divRoundUp(fileSize, MaxSingleIndirectSize);
		if(freeMap->NumClear() < numSectors)
			return FALSE; // not enough space
		
		int remainSize = fileSize;
		for(int i = 0; i < numSectors; i++) {
			dataSectors[i] = freeMap->FindAndSet();
			ASSERT(dataSectors[i] >= 0);

			FileHeader *nextHdr = new FileHeader;
			int nextLevelSize = (remainSize > MaxSingleIndirectSize) ? MaxSingleIndirectSize : remainSize;
			remainSize -= nextLevelSize;

			if(!nextHdr->Allocate(freeMap, nextLevelSize))
				return FALSE;
			else
				nextHdr->WriteBack(dataSectors[i]);
			
			delete nextHdr;
			if(remainSize <= 0)	 break;
		}
	}
	else if(fileSize <= MaxTripleIndirectSize) {
		// Triple indirect allocation
		level = 3;
		numSectors = divRoundUp(fileSize, MaxDoubleIndirectSize);
		if(freeMap->NumClear() < numSectors)
			return FALSE; // not enough space
		
		int remainSize = fileSize;
		for(int i = 0; i < numSectors; i++) {
			dataSectors[i] = freeMap->FindAndSet();
			ASSERT(dataSectors[i] >= 0);

			FileHeader *nextHdr = new FileHeader;
			int nextLevelSize = (remainSize > MaxDoubleIndirectSize) ? MaxDoubleIndirectSize : remainSize;
			remainSize -= nextLevelSize;

			if(!nextHdr->Allocate(freeMap, nextLevelSize))
				return FALSE;
			else
				nextHdr->WriteBack(dataSectors[i]);
			
			delete nextHdr;
			if(remainSize <= 0)	 break;
		}
	}
	else {
		// File size exceeds the maximum size
		return FALSE;
	}

	return TRUE;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void FileHeader::Deallocate(PersistentBitmap *freeMap)
{
	// for (int i = 0; i < numSectors; i++)
	// {
	// 	ASSERT(freeMap->Test((int)dataSectors[i])); // ought to be marked!
	// 	freeMap->Clear((int)dataSectors[i]);
	// }

	// MP4
	if(level == 0) {
		// Direct deallocation
		for(int i = 0; i < numSectors; i++) {
			ASSERT(freeMap->Test((int)dataSectors[i])); // ought to be marked!
			freeMap->Clear((int)dataSectors[i]);
		}
	}
	else {
		// Indirect deallocation
		for(int i = 0; i < numSectors; i++) {
			FileHeader *nextHdr = new FileHeader;
			// Recursively deallocate the next level
			nextHdr->FetchFrom(dataSectors[i]);
			nextHdr->Deallocate(freeMap);
			delete nextHdr;
			ASSERT(freeMap->Test((int)dataSectors[i])); // ought to be marked!
			freeMap->Clear((int)dataSectors[i]);
		}
	}
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk.
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void FileHeader::FetchFrom(int sector)
{
	kernel->synchDisk->ReadSector(sector, (char *)this);

	/*
		MP4 Hint:
		After you add some in-core informations, you will need to rebuild the header's structure
	*/
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk.
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void FileHeader::WriteBack(int sector)
{
	kernel->synchDisk->WriteSector(sector, (char *)this);

	/*
		MP4 Hint:
		After you add some in-core informations, you may not want to write all fields into disk.
		Use this instead:
		char buf[SectorSize];
		memcpy(buf + offset, &dataToBeWritten, sizeof(dataToBeWritten));
		...
	*/
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int FileHeader::ByteToSector(int offset)
{
	// return (dataSectors[offset / SectorSize]);

	// MP4
	if(level == 0) {
		return dataSectors[offset / SectorSize];
	}
	else if(level == 1) {
		int idx = offset / MaxDirectSize;
		FileHeader *nextHdr = new FileHeader;
		nextHdr->FetchFrom(dataSectors[idx]);
		int ret = nextHdr->ByteToSector(offset % MaxDirectSize); //取mod可以得知其在nextHdr裡的offset，遞迴直到找到資料在level 0裡所在的sector
		delete nextHdr;
		return ret;
	}
	else if(level == 2) {
		int idx = offset / MaxSingleIndirectSize;
		FileHeader *nextHdr = new FileHeader;
		nextHdr->FetchFrom(dataSectors[idx]);
		int ret = nextHdr->ByteToSector(offset % MaxSingleIndirectSize);
		delete nextHdr;
		return ret;
	}
	else if(level == 3) {
		int idx = offset / MaxDoubleIndirectSize;
		FileHeader *nextHdr = new FileHeader;
		nextHdr->FetchFrom(dataSectors[idx]);
		int ret = nextHdr->ByteToSector(offset % MaxDoubleIndirectSize);
		delete nextHdr;
		return ret;
	}
	else {
		ASSERT(FALSE);
		return -1;
	}
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int FileHeader::FileLength()
{
	return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void FileHeader::Print()
{
	int i, j, k;
	char *data = new char[SectorSize];

	printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);


    int totalHeaders = 1; // 計算當前層級的 FileHeader
    for (i = 0; i < numSectors; i++) {
        printf("%d ", dataSectors[i]);

		if(this->level > 0) {
			FileHeader nextHdr;
			nextHdr.FetchFrom(dataSectors[i]);
			totalHeaders += nextHdr.CountHeaders();
		}
    }
    printf("\nTotal number of headers: %d\n", totalHeaders);


	printf("\nFile contents:\n");
	for (i = k = 0; i < numSectors; i++)
	{
		kernel->synchDisk->ReadSector(dataSectors[i], data);
		for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++)
		{
			if ('\040' <= data[j] && data[j] <= '\176') // isprint(data[j])
				printf("%c", data[j]);
			else
				printf("\\%x", (unsigned char)data[j]);
		}
		printf("\n");
	}
	delete[] data;
}

// For MP4 bonus2
int FileHeader::CountHeaders() {
    int count = 1; // Count the current header

    if (numBytes > MaxDirectSize && numBytes <= MaxSingleIndirectSize) {
        // Count single indirect headers
        for (int i = 0; i < numSectors; i++) {
            FileHeader nextHdr;
            nextHdr.FetchFrom(dataSectors[i]);
            count += nextHdr.CountHeaders();
        }
    } else if (numBytes > MaxSingleIndirectSize && numBytes <= MaxDoubleIndirectSize) {
        // Count double indirect headers
        for (int i = 0; i < numSectors; i++) {
            FileHeader nextHdr;
            nextHdr.FetchFrom(dataSectors[i]);
            count += nextHdr.CountHeaders();
        }
    } else if (numBytes > MaxDoubleIndirectSize && numBytes <= MaxTripleIndirectSize) {
        // Count triple indirect headers
        for (int i = 0; i < numSectors; i++) {
            FileHeader nextHdr;
            nextHdr.FetchFrom(dataSectors[i]);
            count += nextHdr.CountHeaders();
        }
    }

    return count;
}