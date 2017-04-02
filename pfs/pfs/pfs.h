
#pragma once

#include <string>
#include <vector>

#pragma pack(1)
struct PFXHeaderV1
{
	char               Magic[3];			// "pfX" (pf2)
	unsigned long long RelationFileDataOffset;
    unsigned long      FileNumber;
};

struct PFXEntryV1
{
    unsigned long FileNameLength;
    std::wstring FileName;          // char FileName[1];
    unsigned long Unknown[3];       // = 0x10, 0x0, 0x0
    unsigned long Offset;
    unsigned long Length;
};

struct PFXHeaderV2
{
	char          Magic[3];			// "pfX" (pf2 / pf6)
	unsigned long RelationFileDataOffset;
    unsigned long FileNumber;
};

struct PFXEntryV2
{
    unsigned long FileNameLength;
    std::wstring FileName;          // char FileName[1];
    unsigned long Unknown;          // = 0
    unsigned long Offset;
    unsigned long Length;
};

#pragma pack()

struct PFXEntry
{
    std::wstring FileName;
    unsigned long Offset;
    unsigned long Length;
};

typedef bool (*PFXENTRYEXTRACTER)(HANDLE hFile, std::vector<PFXEntry> & vecEntries);
bool ReadPackgeEntriesV1(HANDLE hFile, std::vector<PFXEntry> & vecEntries);
bool ReadPackgeEntriesV2(HANDLE hFile, std::vector<PFXEntry> & vecEntries);

PFXENTRYEXTRACTER PFXEntryExtracterMap[] = {
    ReadPackgeEntriesV1,
    ReadPackgeEntriesV2,
};