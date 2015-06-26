#include <Windows.h>
#include "resource.h"

HWND hEdit;

struct thread_param
{
	enum {ARR_SIZE = 0x10};
	PTSTR *file_name;
	int file_num;
	bool thread_run;
};

DWORD WINAPI Thread(PVOID pv);
BOOL CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance,
					PSTR pCmdLine, int iCmdShow)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), 0, DlgProc, 0);
	return 0;
}

void AppendMsg(PTSTR szBuffer)
{
	static DWORD dwPos;
	if (0 == szBuffer)
	{
		dwPos = 0;
		SendMessage(hEdit, EM_SETSEL, 0, -1);
		SendMessage(hEdit, EM_REPLACESEL, FALSE, 0);
		return;
	}
	SendMessage(hEdit, EM_SETSEL, (WPARAM)&dwPos, (LPARAM)&dwPos);
	SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)szBuffer);
	SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)TEXT("\r\n"));
	SendMessage(hEdit, EM_GETSEL, 0, (LPARAM)&dwPos);
	return;
}

int mycmp(PTSTR src, PTSTR dst)
{
	int i = 0;
	while (src[i]) tolower(src[i++]);
	return lstrcmp(src, dst);
}
//////////////////////////////////////////////////////////////////////////////////
// 用于展开子目录
// lpszPath		 - 待展开目录
// callback		 - 回调函数
// pcb			 - 回调函数参数
//////////////////////////////////////////////////////////////////////////////////
typedef int (*CallBack)(struct CB* pcb, PTSTR path);

struct CB
{
	thread_param* ptp;
};

int callback(struct CB* pcb, PTSTR path)
{
	if (pcb->ptp->file_num < pcb->ptp->ARR_SIZE)
	{
		lstrcpy((PTSTR)((PBYTE)pcb->ptp->file_name + pcb->ptp->file_num*MAX_PATH), path);
		++pcb->ptp->file_num;
	}
	else
	{
		TCHAR msg[64];
		wsprintf(msg, TEXT("一次拖放少于%d个文件ω"), pcb->ptp->ARR_SIZE);
		AppendMsg(msg);
	}
	return 0;
}
/*
int ExpandDirectory(PTSTR lpszPath, CallBack callback, struct CB* pcb)
{
	static const DWORD MemAllocStep = 1024*MAX_PATH;
	TCHAR			lpFind[MAX_PATH], lpSearch[MAX_PATH], lpPath[MAX_PATH];
	HANDLE			hFindFile;
	WIN32_FIND_DATA FindData;
	int				cnt = 0;

	// Path\*.*
	lstrcpy(lpPath, lpszPath);
	lstrcat(lpPath, TEXT("\\"));
	lstrcpy(lpSearch, lpPath);
	lstrcat(lpSearch, TEXT("*.*"));

	if (INVALID_HANDLE_VALUE != (hFindFile = FindFirstFile(lpSearch, &FindData)))
	{
		do
		{
			// 完整文件名
			lstrcpy(lpFind, lpPath);
			lstrcat(lpFind, FindData.cFileName);

			if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (FindData.cFileName[0] != '.')
					ExpandDirectory(lpFind, callback, pcb);
			}
			else
			{
				callback(pcb, lpFind);
			}
		}while(FindNextFile(hFindFile, &FindData));
		FindClose(hFindFile);
		return 0;
	}
	return -2;
}

//////////////////////////////////////////////////////////////////////////////////
// 添加pInBuf(当它是文件)/pInBuf下的所有子文件(当它是目录)到队列
// pInBuf		 - 待添加文件(展开目录)
// callback		 - 回调函数
// pcb			 - 回调函数参数
//////////////////////////////////////////////////////////////////////////////////
DWORD AppendFileToQueue(PTSTR pInBuf, CallBack callback, struct CB *pcb)
{	
	if (FILE_ATTRIBUTE_DIRECTORY == GetFileAttributes(pInBuf))
	{
		ExpandDirectory(pInBuf, callback, pcb);
	}
	else
	{
		callback(pcb, pInBuf);
	}
	return 0;
}*/

void OnDropFiles(HDROP hDrop, thread_param* ptp)
{
	struct CB cb;
	TCHAR FileName[MAX_PATH];
	DWORD i;
	DWORD FileNum;

	cb.ptp = ptp;

	FileNum  = DragQueryFile(hDrop, -1, NULL, 0);

	for (i=0; i<FileNum; ++i)
	{
		DragQueryFile(hDrop, i, (LPTSTR)FileName, MAX_PATH);
		if (FILE_ATTRIBUTE_DIRECTORY != GetFileAttributes(FileName))
			callback(&cb, FileName);
	}
	DragFinish(hDrop);

	if (!ptp->thread_run)
	{
		CreateThread(NULL, 0, Thread, (PVOID)ptp, 0, NULL);
		ptp->thread_run = true;
	}
	else
		MessageBox(0, TEXT("已添加"), TEXT("提示"), MB_ICONINFORMATION);

	return;
}

BOOL CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static thread_param tp;

	switch (msg)
	{
	case WM_INITDIALOG:
		hEdit = GetDlgItem(hDlg, IDC_EDIT);
		SendMessage(hEdit, EM_LIMITTEXT, -1, 0);
		AppendMsg(TEXT("拖放文件至此处..."));

		tp.file_num = 0;
		if (!(tp.file_name = (PTSTR*)VirtualAlloc(NULL, sizeof(PTSTR*), MEM_COMMIT, PAGE_READWRITE)))
		{
			AppendMsg(TEXT("内存分配错误！"));
			EndDialog(hDlg, 0);
		}
		if (!(*(tp.file_name) = (PTSTR)VirtualAlloc(NULL, tp.ARR_SIZE*MAX_PATH, MEM_COMMIT, PAGE_READWRITE)))
		{
			AppendMsg(TEXT("内存分配错误！"));
			EndDialog(hDlg, 0);
		}

		return TRUE;

	case WM_DROPFILES:
		OnDropFiles((HDROP)wParam, &tp);
		return TRUE;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	}
	return FALSE;
}

int SplitFileNameAndSave(PTSTR cur_dir, PCTSTR file_name, PVOID unpack, DWORD file_length)
{
	int i = 0;
	int len = lstrlen(file_name);
	DWORD ByteWrite;
	TCHAR buf[MAX_PATH], buf2[MAX_PATH], curdir[MAX_PATH];

	GetCurrentDirectory(MAX_PATH, curdir);
	lstrcpy(buf, file_name);
	LPTSTR p = buf, end = buf + len;
	while (p <= end && i < len)
	{
		while(buf[i] != '\\' && buf[i] != '/' && buf[i] != '\0') ++i;
	
		if (i<len)
		{
			buf[i++] = '\0';

			CreateDirectory(p, 0);
//			int err = GetLastError();
			SetCurrentDirectory(p);
//			err = GetLastError();
			p = buf + i;
		}
	}
	HANDLE hFile = CreateFile(p, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		goto SaveErr;

	WriteFile(hFile, unpack, file_length, &ByteWrite, NULL);
	if (ByteWrite != file_length)
	{
		wsprintf(buf2, TEXT("[文件写入错误]%s"), p);
		goto Append;
	}
	if (!GetLastError())
		wsprintf(buf2, TEXT("[已保存]%s"), p);
	else
SaveErr:
	wsprintf(buf2, TEXT("[无法保存]%s"), p);
Append:
	AppendMsg(buf2);
	CloseHandle(hFile);

	SetCurrentDirectory(curdir);
	return 1;
}

/*
struct pfs_head
{
	char				magic[3];			// "pfs"
	unsigned longlong	first_file_offset;	// length of idx in byte
	DWORD				file_number;		// file number
};			sizeof(pfs_head) == 0xF
*/

DWORD WINAPI Thread(PVOID pv)
{
	struct IDX
	{
		unsigned int file_name_len;
		/*** file_name without '\0' ***/   
		/*** 0x10 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0***/
		unsigned int offset;
		unsigned int size;
		
	} idx;
	HANDLE hFile;
	TCHAR szBuffer[MAX_PATH], NewFileName[MAX_PATH], cur_dir[MAX_PATH];
	LPTSTR CurrentFile;
	thread_param * ptp = (thread_param*) pv;
	DWORD ByteRead, dwNowProcess = 0;

	while (dwNowProcess < ptp->file_num)
	{
		CurrentFile = (PTSTR)((PBYTE)ptp->file_name + dwNowProcess*MAX_PATH);

		lstrcpy(cur_dir, CurrentFile);

		int llen = lstrlen(cur_dir);
		while(llen && CurrentFile[llen] != '\\') --llen;
		cur_dir[llen] = '\0';

		lstrcat(cur_dir, TEXT("\\[extract] "));
		lstrcat(cur_dir, CurrentFile + llen + 1);
		CreateDirectory(cur_dir, 0);
		SetCurrentDirectory(cur_dir);

		hFile = CreateFile(CurrentFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			wsprintf(szBuffer, TEXT("无法打开文件, 跳过\r\n%s"), CurrentFile);
			AppendMsg(szBuffer);
			++dwNowProcess;
			continue;
		}

		ReadFile(hFile, szBuffer, 3, &ByteRead, NULL);
		if ((*((int*)szBuffer) & 0x00FFFFFF) != 0x326670)		// "pf2"
		{
			wsprintf(szBuffer, TEXT("文件格式错误, 跳过\r\n%s"), CurrentFile);
			AppendMsg(szBuffer);
			CloseHandle(hFile);
			++dwNowProcess;
			continue;
		}

		DWORD file_pointer = 0xF;		
		DWORD end_of_idx;
		DWORD file_num;
		DWORD extract_file_num = 0;

		ReadFile(hFile, szBuffer, 4, &ByteRead, NULL);
		end_of_idx = *((long*)szBuffer) + 1;			// 偏移要+7

		ReadFile(hFile, &ByteRead, 4, &ByteRead, NULL);		// 或许这是4字？ 高双字
		ReadFile(hFile, &file_num,  4, &ByteRead, NULL);		// 包中包含的文件总数

while (file_pointer < end_of_idx)
{
		SetFilePointer(hFile, file_pointer, NULL, FILE_BEGIN);

		ReadFile(hFile, &idx.file_name_len, 4, &ByteRead, NULL);
		ReadFile(hFile, NewFileName, idx.file_name_len, &ByteRead, NULL);
		NewFileName[idx.file_name_len] = '\0';

		ReadFile(hFile, szBuffer, 0xC, &ByteRead, NULL);		// 11个0x0以及文件名后的0x10

		ReadFile(hFile, &idx.offset,   4, &ByteRead, NULL);
		ReadFile(hFile, &idx.size,   4, &ByteRead, NULL);

		file_pointer += 0x4 + idx.file_name_len + 0x14;		// 下一个索引

		PBYTE content = (PBYTE)VirtualAlloc(NULL, idx.size, MEM_COMMIT, PAGE_READWRITE);
		if (!content)
		{
			AppendMsg(TEXT("内存不足！"));
			CloseHandle(hFile);
			++dwNowProcess;
			continue;
		}

		SetFilePointer(hFile, idx.offset, NULL, FILE_BEGIN);
		ReadFile(hFile, content, idx.size, &ByteRead, NULL);
		
		SplitFileNameAndSave(cur_dir, NewFileName, content, idx.size);

		++extract_file_num;
		VirtualFree(content, idx.size, MEM_DECOMMIT);
		VirtualFree(content, 0, MEM_FREE);		
}
		wsprintf(szBuffer, TEXT("共提取文件(%d/%d)个   --   %s"), extract_file_num, file_num, CurrentFile);
		AppendMsg(szBuffer);

		CloseHandle(hFile);
		++dwNowProcess;
	}
	ptp->file_num   = 0;
	ptp->thread_run = false;
	return 0;
}