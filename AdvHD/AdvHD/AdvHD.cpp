#include <Windows.h>
#include <strsafe.h>


// 没有 magic
struct ARC_HEADER
{
	unsigned long file_num;
	unsigned long idx_length;
};

struct IDX
{
	unsigned long DataSize;
	unsigned long DataOffset;
}; // 后跟wchar_t文件名



int SplitFileNameAndSave(const wchar_t *cur_dir, const wchar_t *file_name, void* unpack, unsigned long  file_length);
extern void AppendMsg(const wchar_t *szBuffer);
extern bool IsPnaFile(const unsigned char *Data);
extern int ExtractPNAPFile(unsigned char *Data, const wchar_t *CurDir, const wchar_t *FileName);



int Entrance(const wchar_t *CurDir, const wchar_t *PackName)
{
	int ret = 0;
	DWORD BytesRead;
	HANDLE hPack;
	struct ARC_HEADER ah;
	struct IDX idx;
	wchar_t MsgBuf[MAX_PATH], FileName[MAX_PATH];
	
	hPack = CreateFile(PackName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if (hPack == INVALID_HANDLE_VALUE)
	{
		StringCchPrintf(MsgBuf, MAX_PATH, L"无法打开文件%s\r\n", PackName);
		AppendMsg(MsgBuf);
		return -1;
	}

	DWORD idx_pointer = sizeof(ARC_HEADER);		
	DWORD end_of_idx;
	DWORD extract_file_num = 0;

	ReadFile(hPack, &ah.file_num, 4, &BytesRead, NULL);
	ReadFile(hPack, &ah.idx_length, 4, &BytesRead, NULL);

	end_of_idx = ah.idx_length + sizeof(ARC_HEADER);

	while (idx_pointer < end_of_idx)
	{
		SetFilePointer(hPack, idx_pointer, NULL, FILE_BEGIN);

		ReadFile(hPack, &idx.DataSize, 4, &BytesRead, NULL);
		ReadFile(hPack, &idx.DataOffset, 4, &BytesRead, NULL);
		idx.DataOffset += end_of_idx;			// 修正偏移地址

		int i = -1;
		do
		{
			ReadFile(hPack, &FileName[++i], 2, &BytesRead, NULL);
		}while (i<MAX_PATH-1 && FileName[i] != '\0');
		FileName[i] = 0;

		idx_pointer += sizeof(struct IDX) + 2 * (1 + wcslen(FileName));

		PVOID Data = VirtualAlloc(NULL, idx.DataSize, MEM_COMMIT, PAGE_READWRITE);
		if (!Data)
		{
			AppendMsg(L"内存不足！\r\n");
			CloseHandle(hPack);
			return -2;
		}

		SetFilePointer(hPack, idx.DataOffset, NULL, FILE_BEGIN);
		ReadFile(hPack, Data, idx.DataSize, &BytesRead, NULL);
		
		if (IsPnaFile((unsigned char*)Data))
		{
			ExtractPNAPFile((unsigned char*)Data, CurDir, FileName);
			++extract_file_num;
		}
		else if (!SplitFileNameAndSave(CurDir, FileName, Data, idx.DataSize))
			++extract_file_num;

		VirtualFree(Data, 0, MEM_RELEASE);
	}
	StringCchPrintf(MsgBuf, MAX_PATH, L"共提取文件(%d/%d)个   --   %s\r\n",
			extract_file_num, ah.file_num, PackName);
	AppendMsg(MsgBuf);
	return 0;
}



static int SplitFileNameAndSave(const wchar_t *cur_dir, const wchar_t *file_name, void* unpack, unsigned long  file_length)
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

	AppendMsg(buf2);
	CloseHandle(hFile);
	return ret;
}
