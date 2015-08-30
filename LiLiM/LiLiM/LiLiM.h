#pragma once
#include <Windows.h>

#pragma pack (1)
typedef struct
{
	DWORD Reserved;
	DWORD FileDataOffset;
	DWORD FileNum;
	char PackageName[0x105];
} PACKHEADER;

#pragma pack ()
typedef struct
{
	char FileName[0x20];
	DWORD FileOffset;
	DWORD FileSize;
} IDX;

extern void AppendMsg(const wchar_t *szBuffer);

int SplitFileNameAndSave(const wchar_t *cur_dir, const wchar_t *file_name, void* unpack, unsigned long file_length);
int Entrance(const wchar_t *PackName, const wchar_t *CurDir);