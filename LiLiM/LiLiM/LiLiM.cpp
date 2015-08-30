#include <Windows.h>
#include <strsafe.h>
#include "LiLiM.h"

static const int MAXPATH = 350;

int SplitFileNameAndSave(const wchar_t *cur_dir, const wchar_t *file_name, void* unpack, unsigned long file_length)
{
	DWORD ByteWrite;
	wchar_t buf[MAXPATH] = {0}, buf2[MAXPATH];

	StringCchCopy(buf, MAXPATH, cur_dir);
	StringCchCat(buf, MAXPATH, L"\\");
	StringCchCat(buf, MAXPATH, file_name);

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
			StringCchPrintf(buf2, MAXPATH, L"[文件创建错误]%s\r\n", file_name);
			ret = -1;
			break;
		}

		WriteFile(hFile, unpack, file_length, &ByteWrite, NULL);

		if (ByteWrite != file_length)
		{
			StringCchPrintf(buf2, MAXPATH, L"[文件写入错误]%s\r\n", file_name);
			ret = -2;
			break;
		}
		int t = GetLastError();
		if (!t || t == ERROR_ALREADY_EXISTS)
			StringCchPrintf(buf2, MAXPATH, L"[已保存]%s\r\n", file_name);
		else
		{
			StringCchPrintf(buf2, MAXPATH, L"[无法保存]%s,错误码%d\r\n", file_name, GetLastError());
			ret = -3;
		}
	}while(0);

	AppendMsg(buf2);
	CloseHandle(hFile);
	return ret;
}

int Entrance(const wchar_t *PackName, const wchar_t *CurDir)
{
	DWORD R, FileSaved = 0;
	PACKHEADER ph;
	IDX idx;
	wchar_t MsgBuf[MAXPATH];

	HANDLE hFile = CreateFile(PackName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		StringCchPrintf(MsgBuf, MAXPATH, L"文件无法打开 %s\r\n", PackName);
		AppendMsg(MsgBuf);
		return -1;
	}

	
	ReadFile(hFile, &ph, sizeof(ph), &R, 0);
	ph.FileNum /= sizeof(IDX);

	for (DWORD i=0; i<ph.FileNum; ++i)
	{
		SetFilePointer(hFile, 0x111 + i * sizeof(idx), 0, FILE_BEGIN);
		ReadFile(hFile, &idx, sizeof(idx), &R, 0);

		PBYTE Data = (PBYTE)VirtualAlloc(NULL, idx.FileSize, MEM_COMMIT, PAGE_READWRITE);
		if (!Data) {
			AppendMsg(L"内存无法分配\r\n");
			return -2;
		}
		// 明文，抽出来直接保存
		SetFilePointer(hFile, ph.FileDataOffset + idx.FileOffset, 0, FILE_BEGIN);
		ReadFile(hFile, Data, idx.FileSize, &R, 0);

		wchar_t UniName[0x30];
		MultiByteToWideChar(932, 0, idx.FileName, -1, UniName, 0x30-1);

		if (!SplitFileNameAndSave(CurDir, UniName, Data, idx.FileSize))
			++FileSaved;

		VirtualFree(Data, 0, MEM_RELEASE);
	}

	if (FileSaved == ph.FileNum)
	{
		StringCchPrintf(MsgBuf, MAXPATH, L"[提取完毕(%d/%d)] %s\r\n", FileSaved, FileSaved, PackName);
		AppendMsg(MsgBuf);
	} else {
		StringCchPrintf(MsgBuf, MAXPATH, L"[提取完毕(%d/%d)] %s\r\n有%d个文件提取失败",
				FileSaved, ph.FileNum, PackName, ph.FileNum - FileSaved);
		MessageBox(0, MsgBuf, 0, MB_ICONWARNING);
	}

	return 0;
}