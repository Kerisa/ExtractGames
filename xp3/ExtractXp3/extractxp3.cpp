#include <Windows.h>
#include "types.h"
#include "cxdec\cxdec.h"
#include "xp3filter_decode.h"
#include "resource.h"

typedef int (*UNCOM)(unsigned char * dst,unsigned long * dstlen,unsigned char * src,unsigned long srclen);

struct xp3_file_header
{
	u8 magic[11]; // = {'\x58', '\x50', '\x33', '\x0D', '\x0A', '\x20', '\0A', '\x1A', '\x8B', '\x67', '\x01'};
	u32 offset_lo;
	u32 offset_hi;
	u32 minor_version;	// 1
	u8 flag;	// 0x80 TVP_XP3_INDEX_CONTINUE
	u32 index_size_lo;
	u32 index_size_hi;
	u32 index_offset_lo;
	u32 index_offset_hi;
};

struct file_entry
{
	u32 crc;
	u32 compress_flag;		// segm
	u32 encryption_flag;	// info
	u64 offset;
	u64 orig_length;
	u64 pkg_length;
	wchar_t file_name[MAX_PATH];
};

struct thread_param
{
	BOOL  IsThreadRunning;
	DWORD FileNum;
	UNCOM unCom;
	char ChooseGame[32];
	TCHAR *FileName;
};

HWND hEdit, hCombo;
HANDLE hThread;
TCHAR CurrentDir[MAX_PATH];

DWORD WINAPI ProcessXp3(PVOID pv);
void OnDropFiles(HDROP hDrop, HWND hwnd, thread_param* ptp);
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

#define MESSAGE(x) MessageBox(0, x, TEXT("提示"), MB_ICONINFORMATION|MB_OK)

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
	SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)TEXT("\n"));
	SendMessage(hEdit, EM_GETSEL, 0, (LPARAM)&dwPos);
	return;
}

u8* get_xp3_idx(HANDLE hFile, u32 *idx_len, UNCOM unCom)
{
	u32 ByteRead;
	xp3_file_header header;

	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	ReadFile(hFile, header.magic, 11, &ByteRead, NULL);
	if (memcmp(header.magic, "XP3\r\n \n\x1A\x8B\x67\x01", 11))
	{
		AppendMsg(TEXT("XP3文件格式错误！"));
		return 0;
	}

	ReadFile(hFile, &header.offset_lo, 4, &ByteRead, NULL);
	ReadFile(hFile, &header.offset_hi, 4, &ByteRead, NULL);
	ReadFile(hFile, &header.minor_version, 4, &ByteRead, NULL);
	ReadFile(hFile, &header.flag, 1, &ByteRead, NULL);
	ReadFile(hFile, &header.index_size_lo, 4, &ByteRead, NULL);
	ReadFile(hFile, &header.index_size_hi, 4, &ByteRead, NULL);
	ReadFile(hFile, &header.index_offset_lo, 4, &ByteRead, NULL);
	ReadFile(hFile, &header.index_offset_hi, 4, &ByteRead, NULL);

	if (header.offset_lo != 0x17 || header.flag != 0x80)
	{
		AppendMsg(TEXT("XP3文件格式错误！"));
		return 0;
	}

	SetFilePointer(hFile, header.index_offset_lo, (PLONG)&header.index_offset_hi, FILE_BEGIN);

	u8  idx_flag;
	u32 idx_size_lo;
	u32 idx_size_hi;
	u32 idx_uncom_lo;
	u32 idx_uncom_hi;

	ReadFile(hFile, &idx_flag, 1, &ByteRead, NULL);
	if (idx_flag != 1) // 未压缩
		AppendMsg(TEXT("文件索引表未压缩"));

	ReadFile(hFile, &idx_size_lo, 4, &ByteRead, NULL);
	ReadFile(hFile, &idx_size_hi, 4, &ByteRead, NULL);
	ReadFile(hFile, &idx_uncom_lo, 4, &ByteRead, NULL);
	ReadFile(hFile, &idx_uncom_hi, 4, &ByteRead, NULL);

	u8 * idx = (u8*)VirtualAlloc(NULL, idx_size_lo, MEM_COMMIT, PAGE_READWRITE);
	u8 * idx_raw = (u8*)VirtualAlloc(NULL, idx_uncom_lo, MEM_COMMIT, PAGE_READWRITE);
	if (!idx || !idx_raw)
	{
		AppendMsg(TEXT("内存分配失败！"));
		return 0;
	}

	ReadFile(hFile, idx, idx_size_lo, &ByteRead, NULL);
	if (idx_flag)
		unCom((PBYTE)idx_raw, &idx_uncom_lo, (PBYTE)idx, idx_size_lo);
	else
		memcpy(idx_raw, idx, idx_size_lo);

	VirtualProtect(idx_raw, idx_uncom_lo, PAGE_READONLY, NULL);
	VirtualFree(idx, idx_size_lo, MEM_FREE);
	*idx_len = idx_uncom_lo;
	return idx_raw;
}

u8* get_file_thunk(u8 *pointer, file_entry *fe, u8 *idx_end)
{
	static const u32 _file = 0x656C6946;
	static const u32 _adlr = 0x726c6461;
	static const u32 _segm = 0x6d676573;
	static const u32 _info = 0x6f666e69;

	if (*((u32*)pointer) != _file)
	{
		AppendMsg(TEXT("文件索引表错误，提取终止"));
		return 0;
	}
	pointer += 4;

	u32 thunk_len = *((u32*)pointer);
	u8* first_end = pointer + thunk_len + 0x8;
	if (first_end > idx_end)
	{
		AppendMsg(TEXT("文件索引表读取错误，提取终止"));	// 反正是不会出现的
		return 0;
	}
	pointer += 0x8;

	while (pointer < first_end){
	switch (*((u32*)pointer))
	{
	default:
		++pointer;
		break;
/*
	case _file:
		pointer += 4;
		first_len = *((u64*)pointer);
		pointer += 8;
		break;
*/
	case _adlr:
		pointer += 0xC;
		fe->crc = *((u32*)pointer);
		pointer += 4;
		break;

	case _segm:
		pointer += 0xC;
		fe->compress_flag = *((u32*)pointer);   // 1 compressed
		pointer += 4;
		fe->offset = *((u64*)pointer);
		pointer += 8;
		fe->orig_length = *((u64*)pointer);
		pointer += 8;
		fe->pkg_length = *((u64*)pointer);
		pointer += 8;
		break;

	case _info:
		pointer += 0x4;
		int info_len = *((u32*)pointer);
		pointer += 0x8;
		fe->encryption_flag = *((u32*)pointer);
		pointer += 0x4;
/*		if (*((u64*)pointer) != fe->orig_length)
			MESSAGE(L"orig_length dismatch");
*/		pointer += 0x8;
/*		if (*((u64*)pointer) != fe->pkg_length)
			MESSAGE(L"pkg_length dismatch");
*/		pointer += 0x8;

		int buf_size = (int)*((u16*)pointer);

		pointer += 0x2;
		lstrcpyW(fe->file_name, (wchar_t*)pointer);
		fe->file_name[buf_size] = '\0';
		break;
	}}
	return pointer;
}

int SplitFileNameAndSave(PTSTR cur_dir, LPCTSTR file_name, PVOID unpack, DWORD file_length)
{
	int i = 0;
	int len = lstrlen(file_name);
	DWORD ByteWrite;
	TCHAR buf[MAX_PATH], buf2[MAX_PATH];;

	SetCurrentDirectory(cur_dir);
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
	return 1;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance,
					PSTR pCmdLine, int iCmdShow)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), 0, DlgProc, 0);
	return 0;
}

BOOL CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static thread_param ThreadParam;
	static HMODULE hZlib;
	TCHAR szBuffer[MAX_PATH];

	switch (msg)
	{
	case WM_INITDIALOG:
		hZlib = LoadLibrary(TEXT("zlib.dll"));
		if (!hZlib)
		{
			MESSAGE(TEXT("缺少zlib.dll文件！"));
			EndDialog(hwnd, 0);
		}
		ThreadParam.unCom = (UNCOM)GetProcAddress(hZlib, "uncompress");

		GetCurrentDirectory(MAX_PATH, CurrentDir);

		hCombo = GetDlgItem(hwnd, IDC_COMBO);
		for (int i=IDS_STRING100; i<=IDS_STRING103; ++i)	// 改为对应游戏数量
		{
			LoadString((HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), i, szBuffer, MAX_PATH);
			SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)szBuffer);
		}

		hEdit = GetDlgItem(hwnd, IDC_EDIT);
		SendMessage(hEdit, EM_LIMITTEXT, -1, 0);
		AppendMsg(TEXT("选择对应游戏后拖放xp3文件到此处..."));
		return TRUE;

	case WM_DROPFILES:
		OnDropFiles((HDROP)wParam, hwnd, &ThreadParam);
		return TRUE;

	case WM_CLOSE:
		if (ThreadParam.IsThreadRunning)
		{
			SuspendThread(hThread);
			if (IDYES == MessageBox(hwnd, TEXT("是否终止提取？"), TEXT("提示"), MB_YESNO|MB_ICONWARNING))
			{
				ThreadParam.IsThreadRunning = FALSE;
				ResumeThread(hThread);
			}
			else
			{
				ResumeThread(hThread);
				break;
			}
		}
		FreeLibrary(hZlib);
		EndDialog(hwnd, 0);
		return TRUE;
	}
	return FALSE;
}

void OnDropFiles(HDROP hDrop, HWND hwnd, thread_param* ThreadParam)
{
	DWORD i, cnt;

	if (-1 == SendMessage(hCombo, CB_GETCURSEL, 0, 0))
	{
		AppendMsg(TEXT("请选择对应游戏！"));
		return;
	}

	if (ThreadParam->IsThreadRunning)
	{
		MESSAGE(TEXT("一次一次来"));
		return;
	}

	ThreadParam->FileNum  = DragQueryFile(hDrop, -1, NULL, 0);
	ThreadParam->FileName = (TCHAR*) VirtualAlloc(NULL, ThreadParam->FileNum * MAX_PATH, MEM_COMMIT, PAGE_READWRITE);
	if (!ThreadParam->FileName)
	{
		AppendMsg(TEXT("内存无法分配！"));
		return;
	}

	for (i=0,cnt=0; i<ThreadParam->FileNum; ++i)
	{
		DragQueryFile(hDrop, i, ThreadParam->FileName + cnt*MAX_PATH, MAX_PATH);
		if (FILE_ATTRIBUTE_DIRECTORY == 
				GetFileAttributes(ThreadParam->FileName + cnt*MAX_PATH))
			continue;
		else
			++cnt;
	}
	DragFinish(hDrop);

	ThreadParam->FileNum = cnt;

	wchar_t buf[32];
	LoadString((HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), 500 + SendMessage(hCombo, CB_GETCURSEL, 0, 0),
				buf, 32);
	WideCharToMultiByte(CP_ACP, 0, buf, 32, ThreadParam->ChooseGame, 32, NULL, NULL);

	if (!(hThread = CreateThread(NULL, 0, ProcessXp3, ThreadParam, 0, NULL)))
	{
		AppendMsg(TEXT("线程创建失败！"));
		return;
	}
	ThreadParam->IsThreadRunning = TRUE;
	return;
}

DWORD WINAPI ProcessXp3(PVOID pv)
{
	DWORD dwNowProcess = 0;
	thread_param *tp;
	TCHAR szFilenameBuf[MAX_PATH], szBuffer[MAX_PATH];

	tp = (thread_param*) pv;
	AppendMsg(0);
	while (dwNowProcess < tp->FileNum)
	{
		int k = lstrlen(tp->FileName);
		while (tp->FileName[k-1] != '\\' && k>0) --k;

		wsprintf(szBuffer, TEXT("[(%u/%u)正在处理]%s"), dwNowProcess+1, tp->FileNum, tp->FileName + dwNowProcess*MAX_PATH + k);
		AppendMsg(szBuffer);

		SetCurrentDirectory(CurrentDir);
		lstrcpy(szFilenameBuf, CurrentDir);
		lstrcat(szFilenameBuf, TEXT("\\[extract] "));
		lstrcat(szFilenameBuf, tp->FileName + dwNowProcess*MAX_PATH + k);

		CreateDirectory(szFilenameBuf, 0);

		HANDLE hFile = CreateFile(tp->FileName + dwNowProcess*MAX_PATH, GENERIC_READ, FILE_SHARE_READ, NULL,
									OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		if (INVALID_HANDLE_VALUE == hFile)
		{
			wsprintf(szBuffer, TEXT("无法打开文件%s，跳过"), tp->FileName + dwNowProcess*MAX_PATH);
			MESSAGE(szBuffer);
			++dwNowProcess;
			continue;
		}

		u8 *xp3_idx, *pidx, *xp3_idx_end;
		u32 xp3_idx_len;

		pidx = xp3_idx = get_xp3_idx(hFile, &xp3_idx_len, tp->unCom);
		if (!pidx)
			break;

		xp3_idx_end = xp3_idx + xp3_idx_len;
	
		pidx += 0x4;
		pidx += *((u32*)pidx) + 0x8;	// skip protection warning
		file_entry fe;

		while(tp->IsThreadRunning && pidx < xp3_idx_end)
		{
			pidx = get_file_thunk(pidx, &fe, xp3_idx_end);
			if (!pidx)
			{
				tp->IsThreadRunning = FALSE;
				break;
			}
			u32 ByteRead;
			u32 offset_hi = (fe.offset >> 32) & 0xFFFFFFFF;
			PBYTE cipher  = (PBYTE)VirtualAlloc(NULL, fe.pkg_length, MEM_COMMIT, PAGE_READWRITE); 
			SetFilePointer(hFile, fe.offset & 0xFFFFFFFF, (PLONG)&offset_hi, FILE_BEGIN);
			ReadFile(hFile, cipher, fe.pkg_length&0xFFFFFFFF, &ByteRead, NULL);

			u8* unpack		  = (u8*) VirtualAlloc(NULL, fe.orig_length, MEM_COMMIT, PAGE_READWRITE);
			u32 unpack_len	  = fe.orig_length&0xFFFFFFFF;
			u32 unpack_offset = 0;

			if (fe.compress_flag)
				tp->unCom(unpack, &unpack_len, cipher, fe.pkg_length);
			else
				memcpy(unpack, cipher, fe.orig_length);

			xp3filter_decode(tp->ChooseGame, fe.file_name, unpack, 
								(u32)(fe.orig_length&0xFFFFFFFF), unpack_offset, 
								fe.orig_length, fe.crc);
	
			SplitFileNameAndSave(szFilenameBuf, fe.file_name, unpack, fe.orig_length);

			VirtualFree(unpack, unpack_len, MEM_DECOMMIT);
			VirtualFree(unpack, 0, MEM_RELEASE);
			VirtualFree(cipher, fe.pkg_length&0xFFFFFFFF, MEM_DECOMMIT);
			VirtualFree(cipher, 0, MEM_RELEASE);
		}
		VirtualFree(xp3_idx, xp3_idx_len, MEM_DECOMMIT);
		VirtualFree(xp3_idx, 0, MEM_RELEASE);
		wsprintf(szBuffer, TEXT("[(%u/%u)提取结束]%s\r\n-----------------------------------------------"),
							dwNowProcess+1, tp->FileNum,tp->FileName + dwNowProcess*MAX_PATH + k);
		AppendMsg(szBuffer);
		++dwNowProcess;
	}
	VirtualFree(tp->FileName, tp->FileNum * MAX_PATH, MEM_DECOMMIT);
	VirtualFree(tp->FileName, 0, MEM_RELEASE);
	tp->IsThreadRunning = FALSE;
	SetCurrentDirectory(CurrentDir);
	return 0;
}