#include <Windows.h>
#include <strsafe.h>
#include "functions.h"

extern HWND g_hEdit;


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
		AppendMsg(g_hEdit, MsgBuf);
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

        // 验证文件名
        if (FileName[0] & 0xff00 || FileName[1] & 0xff00)
        {
            // 不是Unicode字符串
            return -3;
        }

		PVOID Data = VirtualAlloc(NULL, idx.DataSize, MEM_COMMIT, PAGE_READWRITE);
		if (!Data)
		{
			AppendMsg(g_hEdit, L"内存不足！\r\n");
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
	AppendMsg(g_hEdit, MsgBuf);
	return 0;
}
