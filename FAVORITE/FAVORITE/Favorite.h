#pragma once
#include <Windows.h>

#define MAX_PATH1 350

typedef struct
{
	DWORD Magic;
	DWORD OriginalDataLen;
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

// 用于传递需要拆分的bmp数据信息，因为文件保存的路径在外面的函数里。。。。
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
	//SeparateBmp(DWORD h, DWORD w, DWORD b, PBYTE buf, DWORD cch);
	int SetValue(DWORD h, DWORD w, DWORD b, PBYTE buf, DWORD cch);
	DWORD QueryFileNum() {return FileNum;}
	int SaveToFile(const wchar_t *dir, const wchar_t *name);
	~SeparateBmp()
	{
		if (Data) VirtualFree(Data, 0, MEM_RELEASE);
	}
};

class PackageInfo
{
	HANDLE hFile;
	DWORD H_TotalFileNum;
	DWORD IdxOffset;
	DWORD NameOffset;
	DWORD IdxPtr;

	PackageInfo(){}
public:
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

	PackageInfo(HANDLE hFile);
	int GetNextIdx(PPACKAGEINDEX pi, char *out, int cch);
	DWORD FileNum() { return H_TotalFileNum;};
	~PackageInfo() { CloseHandle(hFile); };
};

int GetPackageIndex(HANDLE hFile, PackageInfo::PPACKAGEINDEX* PackageIdx, char** FileNameTable);
int Exactehzc1File(PBYTE PackageData, PBYTE *OriginalData, DWORD PackageDataLen, SeparateBmp & sb);
int MakeBmpFile(PBYTE *RawData, DWORD DataLen, DWORD BppType, DWORD Height, DWORD Width, SeparateBmp & sb);
int Entrance(wchar_t *PackageName, wchar_t *CurrentDir);
int SplitFileNameAndSave(const wchar_t *cur_dir, const wchar_t *file_name, void* unpack, unsigned long file_length);