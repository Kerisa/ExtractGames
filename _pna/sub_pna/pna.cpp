#include <Windows.h>
#include "resource.h"

#define RECORDLIMIT 131072		// 一次处理的文件数上限

HWND hEdit;
CRITICAL_SECTION cs;

struct thread_param
{
	enum {QUEUE_SIZE = 1500};
	PTSTR *queue;
	DWORD front, tail;
	HANDLE hEvent;
	HANDLE hThread;
	bool thread_exit;
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
	lstrcat(szBuffer, TEXT("\r\n"));
	SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)szBuffer);
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
	int cnt;
	thread_param* ptp;
	PTSTR filter;
};

int callback(struct CB* pcb, PTSTR path)
{
	int len = lstrlen(path);
	while(len>=0 && path[len-1] != '.') --len;
	if (!lstrcmp(&path[len], pcb->filter))
	{
		while (pcb->ptp[pcb->cnt].front == pcb->ptp[pcb->cnt].tail+1)		// 队列满，转下一个
			pcb->cnt = (pcb->cnt+1)%4;

		EnterCriticalSection(&cs);
		{
			lstrcpy((PTSTR)((PBYTE)*pcb->ptp[pcb->cnt].queue + pcb->ptp[pcb->cnt].tail*MAX_PATH), path);
		
			if (pcb->ptp[pcb->cnt].tail == pcb->ptp[pcb->cnt].front)		// 原先队列为空，置位
				SetEvent(pcb->ptp[pcb->cnt].hEvent);

			pcb->ptp[pcb->cnt].tail = (pcb->ptp[pcb->cnt].tail + 1) % pcb->ptp[pcb->cnt].QUEUE_SIZE;// 更新队列
		}
		LeaveCriticalSection(&cs);

		pcb->cnt = (pcb->cnt+1)%4;	// 转下一个线程
	}
	return 0;
}

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
}

void OnDropFiles(HDROP hDrop, HWND hwnd, thread_param* ptp)
{
	struct CB cb;
	TCHAR FileName[MAX_PATH];
	DWORD i;
	DWORD FileNum;

	cb.cnt = 0;
	cb.filter = TEXT("pna");
	cb.ptp = ptp;

	FileNum  = DragQueryFile(hDrop, -1, NULL, 0);

	for (i=0; i<FileNum; ++i)
	{
		DragQueryFile(hDrop, i, (LPTSTR)FileName, MAX_PATH);
		AppendFileToQueue(FileName, callback, &cb);
	}
	DragFinish(hDrop);

	return;
}

BOOL CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static thread_param tp[4];

	switch (msg)
	{
	case WM_INITDIALOG:
		hEdit = GetDlgItem(hDlg, IDC_EDIT);
		SendMessage(hEdit, EM_LIMITTEXT, -1, 0);
		AppendMsg(TEXT("拖放pna文件至此处..."));

		for (int i=0; i<4; ++i)
		{
			if (!(tp[i].hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)))
			{
				AppendMsg(TEXT("事件初始化错误！"));
				EndDialog(hDlg, 0);
			}
			if (!(tp[i].queue = (PTSTR*)VirtualAlloc(NULL, sizeof(PTSTR*), MEM_COMMIT, PAGE_READWRITE)))
			{
				AppendMsg(TEXT("内存分配错误！"));
				EndDialog(hDlg, 0);
			}
			if (!(*(tp[i].queue) = (PTSTR)VirtualAlloc(NULL, tp[i].QUEUE_SIZE*MAX_PATH, MEM_COMMIT, PAGE_READWRITE)))
			{
				AppendMsg(TEXT("内存分配错误！"));
				EndDialog(hDlg, 0);
			}
			if (!(tp[i].hThread = CreateThread(NULL, 0, Thread, &tp[i], 0, NULL)))
			{
				AppendMsg(TEXT("线程创建失败！"));
				EndDialog(hDlg, 0);
			}
			tp[i].front = tp[i].tail = 0;
			tp[i].thread_exit = false;
		}
		InitializeCriticalSection(&cs);
		return TRUE;

	case WM_DROPFILES:
		OnDropFiles((HDROP)wParam, hDlg, tp);
		return TRUE;

	case WM_CLOSE:
		for (int i=0; i<4; ++i)
		{
			tp[i].thread_exit = true;
			SetEvent(tp[i].hEvent);
		}

		EndDialog(hDlg, 0);
		return TRUE;
	}
	return FALSE;
}

struct PNA_HEAD
{
	char magic[4];
	DWORD unkonwn1;
	DWORD unkonwn2;
	DWORD unkonwn3;
	DWORD file_num_fake;
	DWORD next_type;
};

struct PNA_IDX
{
	int file_serial;
	int h_offset;
	int v_offset;
	int width;
	int height;
	int unknown1;
	int unknown2;
	int unknown3;
	int file_size;
	int next_type;	// 00:下个是正常的文件 01:下个是无效的FFFF 02:再经过一个0x24的idx(FFFF)结束
};

const TCHAR pos_title[] = TEXT("ID\th_offset\tv_offset\twidth\theight\r\n");

DWORD WINAPI Thread(PVOID pv)
{
	HANDLE hFile;
	TCHAR szBuffer[MAX_PATH], fmt[MAX_PATH];
	LPTSTR CurrentFile;
	thread_param * ptp = (thread_param*) pv;
	DWORD ByteRead, dwNowProcess = 0, dwFileProcessed = 0;
	int cnt = 0;

	while (1)
	{
		WaitForSingleObject(ptp->hEvent, INFINITE);

		if (ptp->thread_exit) break;

		CurrentFile = (PTSTR)((PBYTE)*ptp->queue + ptp->front*MAX_PATH);

		lstrcpy(fmt, CurrentFile);

		int llen = lstrlen(fmt);
		while(llen && fmt[llen] != '.') --llen;
		fmt[llen] = '\0';

		lstrcat(fmt, TEXT("_%02d.png"));

		hFile = CreateFile(CurrentFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			wsprintf(szBuffer, TEXT("无法打开文件, 跳过\r\n%s"), CurrentFile);
			AppendMsg(szBuffer);
			++dwNowProcess;
			continue;
		}

		struct PNA_HEAD pna_head;

		ReadFile(hFile, &pna_head, sizeof(pna_head), &ByteRead, NULL);

//////////////////////////////////////////////////////////////////
		TCHAR tmp[3];
		tmp[0] = CurrentFile[llen]; tmp[1] = CurrentFile[llen+1]; tmp[2] = CurrentFile[llen+2];
		CurrentFile[llen+1] = CurrentFile[llen+3] = 't'; CurrentFile[llen+2] = 'x';

		struct PNA_IDX *pna_idx = (struct PNA_IDX*)VirtualAlloc(NULL, sizeof(PNA_IDX)*pna_head.file_num_fake, MEM_COMMIT, PAGE_READWRITE);
		DWORD  png_ptr = sizeof(PNA_HEAD) + sizeof(PNA_IDX) * pna_head.file_num_fake - 4;

		ReadFile(hFile, pna_idx, sizeof(PNA_IDX)*pna_head.file_num_fake, &ByteRead, 0);

		HANDLE hIdx = CreateFile(CurrentFile, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		WriteFile(hIdx, pos_title, sizeof(pos_title)/sizeof(pos_title[0])-sizeof(TCHAR), &ByteRead, 0);		// 最后的\0不用写

		struct PNA_IDX *p = pna_idx;
		for (int i=0; i<pna_head.file_num_fake; ++i, ++p)
		{
			if (p->file_serial == -1) continue;
			++cnt;

			wsprintf(szBuffer, TEXT("%d\t\t\t%d\t\t\t%d\t\t\t%d\t\t\t%d\r\n"),
							p->file_serial, p->h_offset, p->v_offset, p->width, p->height);
			WriteFile(hIdx, szBuffer, lstrlen(szBuffer), &ByteRead, 0);

			PBYTE FileBuf = (PBYTE)VirtualAlloc(NULL, p->file_size, MEM_COMMIT, PAGE_READWRITE);
			SetFilePointer(hFile, png_ptr, 0, FILE_BEGIN);
			ReadFile(hFile, FileBuf, p->file_size, &ByteRead, NULL);

			wsprintf(szBuffer, fmt, p->file_serial);
			HANDLE hSave = CreateFile(szBuffer, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			WriteFile(hSave, FileBuf, p->file_size, &ByteRead, 0);
			CloseHandle(hSave);
			++dwFileProcessed;

			VirtualFree(FileBuf, p->file_size, MEM_DECOMMIT);
			VirtualFree(FileBuf, 0, MEM_RELEASE);

			png_ptr += p->file_size;
		}
		CloseHandle(hIdx);
		CloseHandle(hFile);

		CurrentFile[llen+1] = 'p'; CurrentFile[llen+2] = 'n'; CurrentFile[llen+3] = 'a';
		wsprintf(szBuffer, TEXT("===[处理完成(%d/%d)] %s==="), dwFileProcessed, cnt, CurrentFile);
		AppendMsg(szBuffer);
/////////////////////////////////////////////////////////////////////

		EnterCriticalSection(&cs);
		{
			ptp->front = (ptp->front + 1) % ptp->QUEUE_SIZE;
		
			if (ptp->front == ptp->tail)
				ResetEvent(ptp->hEvent);
		}
		LeaveCriticalSection(&cs);
	}
	return 0;
}