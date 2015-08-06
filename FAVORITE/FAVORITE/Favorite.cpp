#include <Windows.h>
#include "Favorite.h"

_UNCOM gfnUncompress;

int GetPackageIndex(HANDLE hFile, PPACKAGEINDEX* PackageIdx, char** FileNameTable)
{
	PACKAGEHEADER PackHeader;
	DWORD R, SizeOfIdx, SizeOfName;
	PBYTE FNT, PI;

	*PackageIdx = 0;
	*FileNameTable = 0;
	if (hFile == INVALID_HANDLE_VALUE)
	{
		AppendMsg(L"无效的文件句柄！\r\n");
		return 0;
	}

	SetFilePointer(hFile, 0, 0, FILE_BEGIN);
	ReadFile(hFile, &PackHeader, sizeof(PackHeader), &R, 0);

	SizeOfIdx = PackHeader.TotalFileNum * sizeof(PACKAGEINDEX);
	if (!(PI = (PBYTE)VirtualAlloc(0, SizeOfIdx, MEM_COMMIT, PAGE_READWRITE)))
	{
		AppendMsg(L"内存无法分配\r\n");
		return 0;
	}

	ReadFile(hFile, PI, SizeOfIdx, &R, 0);

	SizeOfName = ((PPACKAGEINDEX)PI)[0].FileOffset - SizeOfIdx - sizeof(PACKAGEHEADER);
	if (!(FNT = (PBYTE)VirtualAlloc(0, SizeOfName, MEM_COMMIT, PAGE_READWRITE)))
	{
		AppendMsg(L"内存无法分配\r\n");
		return 0;
	}

	ReadFile(hFile, FNT, SizeOfName, &R, 0);

	*PackageIdx = (PPACKAGEINDEX)PI;
	*FileNameTable = (char*)FNT;
	return PackHeader.TotalFileNum;
}

int Exactehzc1File(PBYTE PackageData, PBYTE *OriginalData, DWORD PackageDataLen) /*需要调用 gfnUncompress*/
{
	Phzc1HEADER Hzc1 = (Phzc1HEADER)PackageData;
	PNVSGHEADER Nvsg = (PNVSGHEADER)(PackageData + sizeof(hzc1HEADER));

	DWORD OriginalLen = Hzc1->OriginalFileLen;

	if (Nvsg->magic != 0x4753564E)	// "NVSG"
	{
		AppendMsg(L"NVSG标志符不匹配!\r\n");
		return -1;
	}

	if (!gfnUncompress) return 0;
	PBYTE RawData = (PBYTE)VirtualAlloc(NULL, OriginalLen + sizeof(BmpHeader), MEM_COMMIT, PAGE_READWRITE);
	gfnUncompress(RawData + sizeof(BmpHeader),
				  &OriginalLen,
				  PackageData + Hzc1->FileInfoLen + sizeof(hzc1HEADER),
				  PackageDataLen - Hzc1->FileInfoLen - sizeof(hzc1HEADER));

	MakeBmpFile(RawData, Hzc1->OriginalFileLen, Nvsg->BppType, Nvsg->Height, Nvsg->Width);

	*OriginalData = RawData;
	return Hzc1->OriginalFileLen + 54;
}

int MakeBmpFile(PBYTE RawData, DWORD FileLen, DWORD BppType, DWORD Height, DWORD Width)
{
	bool bNeedSeparate = false;
	memcpy(RawData, BmpHeader, sizeof(BmpHeader));

	BYTE Bpp;
	if (BppType == 0)		Bpp = 24;
	else if (BppType == 1)	Bpp = 32;
	else if (BppType == 2) {Bpp = 32;	bNeedSeparate = true; }
	else if (BppType == 3) {Bpp = 8;	AppendMsg(L"BppType 3\r\n"); }
	else
	{
		AppendMsg(L"未知颜色位数\r\n");
		return 0;
	}
	if (!bNeedSeparate)
	{
		*(PDWORD)(RawData + 0x2)  = FileLen + 54;
		*(PDWORD)(RawData + 0x22) = FileLen;
		*(PDWORD)(RawData + 0x12) = Width;
		*(PDWORD)(RawData + 0x16) = -Height;
		*(PBYTE)(RawData + 0x1C) = Bpp;

		PDWORD p = (PDWORD)(RawData + 54);
		for (int i=0; i<(FileLen>>2); ++i)	// 把Alpha通道为透明(0x0)的像素换成白色
			if (!(p[i] & 0xff000000))
				p[i] = 0x00ffffff;
	}
	else
	{

	}

	return 0;
}

int Enterence(wchar_t *PackageName, wchar_t *CurrentDir)
{
	wchar_t szBuf[MAX_PATH];
	DWORD R, SavedFileNum = 0;

	if (!PackageName) return 0;
	HANDLE hFile = CreateFile(PackageName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		wsprintf(szBuf, L"无法打开文件%s, 错误码%d\r\n", PackageName, GetLastError());
		AppendMsg(szBuf);
		return 0;
	}

	PPACKAGEINDEX PackIdx;
	char *FileNameTable;
	DWORD TotFileNum = GetPackageIndex(hFile, &PackIdx, &FileNameTable);

	for (int i=0; i<TotFileNum; ++i)
	{
		wchar_t szFileNameBuf[MAX_PATH];
		PBYTE PackData = (PBYTE)VirtualAlloc(NULL, PackIdx[i].FileLength, MEM_COMMIT, PAGE_READWRITE);
		if (!PackData)
		{
			AppendMsg(L"内存无法分配，内存不够了么？\r\n");
			return 0;
		}
		
		SetFilePointer(hFile, PackIdx[i].FileOffset, 0, FILE_BEGIN);
		ReadFile(hFile, PackData, PackIdx[i].FileLength, &R, 0);

		// Code Page	932	shift_jis	ANSI/OEM Japanese; Japanese (Shift-JIS)
		MultiByteToWideChar(932, 0, &FileNameTable[PackIdx[i].FileNameOffset], -1, szFileNameBuf, MAX_PATH-1);
		if (*(PDWORD)PackData == 0x5367674F || *(PDWORD)PackData == 0x46464952)	// "OggS" "RIFF"
		{
			if (*(PDWORD)PackData == 0x5367674F)
				wcscat(szFileNameBuf, L".ogg");
			else
				wcscat(szFileNameBuf, L".wav");

			if (!SplitFileNameAndSave(CurrentDir, szFileNameBuf, PackData, PackIdx[i].FileLength))
				++SavedFileNum;
			VirtualFree(PackData, 0, MEM_RELEASE);
		}
		else if (*(PDWORD)PackData == 0x31637A68)	// "hzc1"
		{
			PBYTE OriginalData = 0;
			DWORD FileLen = 0;
			wcscat(szFileNameBuf, L".bmp");
			if (!(FileLen = Exactehzc1File(PackData, &OriginalData, PackIdx[i].FileLength)))
			{
				wsprintf(szBuf, L"文件解码失败！ - %s\r\n", &FileNameTable[PackIdx[i].FileNameOffset]);
				AppendMsg(szBuf);
			}
			else if (!SplitFileNameAndSave(CurrentDir, szFileNameBuf, OriginalData, FileLen))
				++SavedFileNum;
			VirtualFree(OriginalData, 0, MEM_RELEASE);
		}
	}
	if (TotFileNum == SavedFileNum)
	{
		wsprintf(szBuf, L"[提取完成(%d/%d)] %s\r\n", SavedFileNum, SavedFileNum, PackageName);
		AppendMsg(szBuf);
	}
	else
	{
		wsprintf(szBuf, L"[提取完成(%d/%d)] %s\r\n有%d个文件出错",
					SavedFileNum, TotFileNum, PackageName, TotFileNum-SavedFileNum);
		MessageBox(0, szBuf, L"提示", MB_ICONWARNING);
	}
	VirtualFree(PackIdx, 0, MEM_RELEASE);
	VirtualFree(FileNameTable, 0, MEM_RELEASE);
	CloseHandle(hFile);
	return 0;
}

int SplitFileNameAndSave(const wchar_t *cur_dir, const wchar_t *file_name, void* unpack, unsigned long file_length)
{
	DWORD ByteWrite;
	wchar_t buf[MAX_PATH] = {0}, buf2[MAX_PATH];

	lstrcpyW(buf, cur_dir);
	lstrcatW(buf, L"\\");
	lstrcatW(buf, file_name);

	int len = lstrlenW(buf);
	int i = lstrlenW(cur_dir) + 1;
	wchar_t *p = buf, *end = buf + len;
	while (p <= end && i < len)
	{
		while(buf[i] != '\\' && buf[i] != '/' && buf[i] != '\0') ++i;
	
		if (i<len)
		{
			wchar_t tmp = buf[i];
			buf[i] = '\0';

			CreateDirectoryW(p, 0);
			buf[i] = tmp;
			++i;
			p = buf + i;
		}
	}

	HANDLE hFile;
	int ret = 0;
	do{
		hFile = CreateFile(p, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			wsprintfW(buf2, L"[文件创建错误]%s\r\n", p);
			ret = -1;
			break;
		}

		WriteFile(hFile, unpack, file_length, &ByteWrite, NULL);

		if (ByteWrite != file_length)
		{
			wsprintfW(buf2, L"[文件写入错误]%s\r\n", p);
			ret = -2;
			break;
		}
		int t = GetLastError();
		if (!t || t == ERROR_ALREADY_EXISTS)
			wsprintfW(buf2, L"[已保存]%s\r\n", p);
		else
		{
			wsprintfW(buf2, L"[无法保存]%s,错误码%d\r\n", p, GetLastError());
			ret = -3;
		}
	}while(0);

	AppendMsg(buf2);
	CloseHandle(hFile);
	return ret;
}