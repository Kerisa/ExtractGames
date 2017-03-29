
#pragma once

#include <string>

#pragma pack(1)
struct pfs_head
{
	char          Magic[3];			// "pfX"
	unsigned long RelationFileDataOffset;
    unsigned long FileNumber;
};

struct pfs_entry
{
    unsigned long FileNameLength;
    std::wstring FileName;           // char FileName[1];
    unsigned long Unknown;      // = 0
    unsigned long Offset;
    unsigned long Length;
};
#pragma pack()