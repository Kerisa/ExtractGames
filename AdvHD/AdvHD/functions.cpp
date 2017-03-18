

#include "functions.h"

#include <Windows.h>
#include <strsafe.h>

extern HWND g_hEdit;

void AppendMsg(HWND hOut, const wchar_t *szBuffer)
{
	static DWORD dwPos;
	if (0 == szBuffer)
	{
		dwPos = 0;
		SendMessage(hOut, EM_SETSEL, 0, -1);
		SendMessage(hOut, EM_REPLACESEL, FALSE, 0);
	} else {
		SendMessage(hOut, EM_SETSEL, (WPARAM)&dwPos, (LPARAM)&dwPos);
		SendMessage(hOut, EM_REPLACESEL, 0, (LPARAM)szBuffer);
		SendMessage(hOut, EM_GETSEL, 0, (LPARAM)&dwPos);
	}
	return;
}


int SplitFileNameAndSave(const wchar_t *cur_dir, const wchar_t *file_name, void* unpack, unsigned long  file_length)
{
	DWORD ByteWrite;
	wchar_t buf[MAX_PATH] = {0}, buf2[MAX_PATH];

	StringCchCopy(buf, MAX_PATH, cur_dir);
	StringCchCat (buf, MAX_PATH, L"\\");
	StringCchCat (buf, MAX_PATH, file_name);

	int len = wcslen(buf);
	int i = wcslen(cur_dir) + 1;
	wchar_t *p = buf, *end = buf + len;
	while (p <= end && i < len)
	{
		while(buf[i] != '\\' && buf[i] != '/' && buf[i] != '\0') ++i;
		if (buf[i] == '/') buf[i] = '\\';
		if (i<len)
		{
			wchar_t tmp = buf[i];
			buf[i] = '\0';

			CreateDirectoryW(p, 0);
			buf[i] = tmp;
			++i;
		}
	}

	HANDLE hFile;
	int ret = 0;
	do{
		hFile = CreateFile(buf, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			StringCchPrintf(buf2, MAX_PATH, L"[文件创建错误]%s\r\n", file_name);
			ret = -1;
			break;
		}

		WriteFile(hFile, unpack, file_length, &ByteWrite, NULL);

		if (ByteWrite != file_length)
		{
			StringCchPrintf(buf2, MAX_PATH, L"[文件写入错误]%s\r\n", file_name);
			ret = -2;
			break;
		}
		
		int t = GetLastError();
		if (!t || t == ERROR_ALREADY_EXISTS)
			StringCchPrintf(buf2, MAX_PATH, L"[已保存]%s\r\n", file_name);
		else
		{
			StringCchPrintf(buf2, MAX_PATH, L"[无法保存]%s\r\n", file_name);
			ret = -3;
		}
	}while(0);

	AppendMsg(g_hEdit, buf2);
	CloseHandle(hFile);
	return ret;
}
