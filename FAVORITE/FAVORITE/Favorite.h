#pragma once
#include <Windows.h>

typedef struct
{
	DWORD TotalFileNum;
	DWORD unKnown;
} PACKAGEHEADER, *PPACKAGEHEADER;

typedef struct
{
	DWORD FileNameOffset;
	DWORD FileOffset;
	DWORD FileLength;
} PACKAGEINDEX, *PPACKAGEINDEX;

typedef struct
{
	DWORD Magic;
	DWORD OriginalFileLen;
	DWORD FileInfoLen;
} hzc1HEADER, *Phzc1HEADER;

typedef struct
{
	DWORD magic;
	WORD unKnown1;	// 0x100
	WORD BppType;
	WORD Width;
	WORD Height;
	DWORD unKnown2;	// 0
	DWORD unKnown3;	// 0
	DWORD unKnown4;	// 0
	DWORD unKnown5;	// 0
	DWORD unKnown6;	// 0
} NVSGHEADER, *PNVSGHEADER;

typedef int (*_UNCOM)(unsigned char *,unsigned long *,unsigned char *,unsigned long);

static const unsigned char BmpHeader [] = {
	0x42, 0x4D, 0x36, 0x00, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 
	0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

extern void AppendMsg(const wchar_t *szBuffer);

class SeparateBmp
{
	enum {MAXPATH = 350};
	DWORD FileNum;
	DWORD Width;
	DWORD Height;
	DWORD Bpp;
	DWORD FileSize;
	PBYTE Data;

	SeparateBmp(SeparateBmp&){};
public:
	SeparateBmp():Data(0), FileNum(0){};
	SeparateBmp(DWORD h, DWORD w, DWORD b, PBYTE buf, DWORD cch);
	int SaveToFile(const wchar_t *dir, const wchar_t *name);
	~SeparateBmp();
};

SeparateBmp::SeparateBmp(DWORD h, DWORD w, DWORD b, PBYTE buf, DWORD cch)
			:Height(h), Width(w), Bpp(b)
{
	FileSize = h * w * (b >> 3);
	FileNum  = cch / FileSize;
	FileSize += 54;	// 加上BMP文件头

	PBYTE p = buf, p_end = buf + cch;

	if (Data = (BYTE*)VirtualAlloc(NULL, cch, MEM_COMMIT, PAGE_READWRITE))
		memcpy(Data, buf, cch);
	else
		AppendMsg(L"SeparateBmp中内存分配失败！\r\n");
}

int SeparateBmp::SaveToFile(const wchar_t *dir, const wchar_t *NameWithoutSuffix)
{
	DWORD R;
	wchar_t format[MAXPATH] = {0};
	wchar_t newname[MAXPATH] = {0};
	wchar_t buf[MAXPATH] = {0};
	BYTE bmp[sizeof(BmpHeader)];
	wcscpy(format, dir);
	wcscat(format, L"\\");
	wcscat(format, NameWithoutSuffix);
	wcscat(format, L"_%3d.bmp");

	memcpy(bmp, BmpHeader, sizeof(BmpHeader));
	*(PDWORD)(bmp +  0x2) = FileSize;
	*(PDWORD)(bmp + 0x22) = FileSize - 54;
	*(PDWORD)(bmp + 0x12) = Width;
	*(PDWORD)(bmp + 0x16) = -Height;
	*(PBYTE) (bmp + 0x1C) = Bpp;

	DWORD FileSaved = 0;
	for (int i=0; i<FileNum; ++i)
	{
		wsprintf(newname, format, i);
		HANDLE hSave = CreateFile(newname, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (hSave == INVALID_HANDLE_VALUE)
		{
			wsprintf(buf, L"[创建失败] %s\r\n", newname);
			AppendMsg(buf);
			continue;
		}
		
		if (!WriteFile(hSave, bmp, sizeof(bmp), &R, 0) ||
			!WriteFile(hSave, Data+i*(FileSize-54), FileSize-54, &R, 0)
			)
		{
			wsprintf(buf, L"[写入失败] %s\r\n", newname);
			AppendMsg(buf);
		}
		else ++FileSaved;

		CloseHandle(hSave);
	}
	return FileSaved;
}

int GetPackageIndex(HANDLE hFile, PPACKAGEINDEX* PackageIdx, char** FileNameTable);
int Exactehzc1File(PBYTE PackageData, PBYTE *OriginalData, DWORD PackageDataLen);
int MakeBmpFile(PBYTE RawData, DWORD FileLen, DWORD BppType, DWORD Height, DWORD Width);
int Enterence(wchar_t *PackageName, wchar_t *CurrentDir);
int SplitFileNameAndSave(const wchar_t *cur_dir, const wchar_t *file_name, void* unpack, unsigned long file_length);