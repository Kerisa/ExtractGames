#include <Windows.h>
#include "dsc.h"
#include "cbg.h"
#include "arc.h"
#include "resource.h"

#define MAX_PATH 400

HWND hEdit;
CRITICAL_SECTION cs;

struct thread_param
{
	enum {QUEUE_SIZE = 1500};
	wchar_t **queue;
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
	lstrcat(szBuffer, TEXT("\r\n"));
	SendMessage(hEdit, EM_SETSEL, (WPARAM)&dwPos, (LPARAM)&dwPos);
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
	if (!pcb->filter || !lstrcmp(&path[len], pcb->filter))
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
	cb.filter = TEXT("arc");
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
		AppendMsg(TEXT("拖放arc文件至此处...\r\n【注意】文件路径须小于200个字符\r\n"));

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
			if (!(*(tp[i].queue) = (wchar_t*)VirtualAlloc(NULL, tp[i].QUEUE_SIZE*MAX_PATH, MEM_COMMIT, PAGE_READWRITE)))
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

extern GET_IDX get_arc_idx[ARC_FILE_TYPE];

DWORD WINAPI Thread(PVOID pv)
{
	HANDLE hFile;
	struct IDX *arc_idx;
	TCHAR szBuffer[MAX_PATH], cur_dir[MAX_PATH];
	LPTSTR CurrentFile;
	DWORD dwNowProcess = 0;
	int ret;
	thread_param * ptp = (thread_param*) pv;
	

	while (1)
	{
		WaitForSingleObject(ptp->hEvent, INFINITE);

		if (ptp->thread_exit) break;

		CurrentFile = (PTSTR)((PBYTE)*ptp->queue + ptp->front*MAX_PATH);

		lstrcpy(cur_dir, CurrentFile);

		int l = lstrlen(cur_dir);
		while(l && cur_dir[l-1] != '\\') --l;
		cur_dir[l] = '\0';

		lstrcat(cur_dir, TEXT("[extract] "));
		lstrcat(cur_dir, &CurrentFile[l]);
		CreateDirectory(cur_dir, 0);
		
		do{
			arc_idx = 0;
			hFile = CreateFile(CurrentFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				wsprintf(szBuffer, TEXT("无法打开文件, 跳过\r\n%s"), CurrentFile);
				AppendMsg(szBuffer);
				break;
			}
			int i = Is_arc(hFile);
			if (i == ARC_FILE_TYPE)
			{
				wsprintf(szBuffer, TEXT("错误的arc文件:%s"), CurrentFile);
				AppendMsg(szBuffer);
				break;
			}

			u32 file_num = 0;
			u32 idx_size = 0;
			struct IDX *arc_idx = get_arc_idx[i](hFile, &file_num, &idx_size);

			if (!file_num || !arc_idx)
			{
				wsprintf(szBuffer, TEXT("文件读取错误\r\n%s"), CurrentFile);
				AppendMsg(szBuffer);
				break;
			}

			if (file_num == (ret = arc_extract_file_save(hFile, arc_idx, file_num, idx_size, cur_dir)))
			{
				wsprintf(szBuffer, TEXT("[提取完成(%d/%d)]%s"), ret, file_num, CurrentFile);
				AppendMsg(szBuffer);
			}
			else
			{
				wsprintf(szBuffer, TEXT("提取%d个文件，共%d个，有%d个发生错误\r\n%s"),
								ret, file_num, file_num-ret, CurrentFile);
				MessageBox(0, szBuffer, TEXT("提示"), MB_ICONWARNING);
			}
		}while(0);

		if (arc_idx) free(arc_idx);
		if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);

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