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

int SplitFileNameAndSave(PTSTR cur_dir, char *file_name, PVOID unpack, DWORD file_length)
{
	int i = 0;
	int len = lstrlen(file_name);
	DWORD ByteWrite;
	TCHAR buf[MAX_PATH] = {0}, buf2[MAX_PATH], curdir[MAX_PATH];

//	WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, file_name, lstrlenW(file_name), buf, MAX_PATH, 0, 0);
	lstrcpy(buf, file_name);

	GetCurrentDirectory(MAX_PATH, curdir);

	PTSTR p = buf, end = buf + len;
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

int DecoderACP(PBYTE in, DWORD in_length, PBYTE out, int out_size);

DWORD WINAPI Thread(PVOID pv)
{
	struct BIN_HEAD
	{
		char magic[8];				// "ESC-ARC2"
		unsigned long key;
		unsigned long file_num;
		unsigned long length_of_file_name;
	} bin_head;

	struct IDX
	{
		unsigned long file_name_offset;
		unsigned long offset;
		unsigned long size;
	};

	struct INFO
	{
		DWORD	unknown;		// 1
		HANDLE	hFile;			// 打开的bin文件句柄
		unsigned long offset;	// 其实就是idx的内容
		unsigned long size;		// 其实就是idx的内容
	}info;

	PBYTE pIDX, pNAME;

	HANDLE hFile;
	TCHAR szBuffer[MAX_PATH], cur_dir[MAX_PATH];
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

		ReadFile(hFile, &bin_head, sizeof(bin_head), &ByteRead, NULL);		// sizeof(bin_head) == 0x14

		if (memcmp("ESC-ARC2", bin_head.magic, 8))
		{
			wsprintf(szBuffer, TEXT("文件格式错误, 跳过\r\n%s"), CurrentFile);
			CloseHandle(hFile);
			AppendMsg(szBuffer);
			++dwNowProcess;
			continue;
		}

		DWORD key_a, key_c, key_d, key_s;

		key_c = bin_head.key;

		key_c ^= 0x65AC9365;
		key_d = ((key_c << 1) ^ key_c) << 3;
		key_a = ((key_c >> 1) ^ key_c) >> 3;
		key_a ^= (key_d ^ key_c);
		bin_head.file_num ^= key_a;

		key_a ^= 0x65AC9365;
		key_s = ((key_a >> 1) ^ key_a) >> 3;
		key_c = ((key_a << 1) ^ key_a) << 3;
		key_s ^= (key_c ^ key_a);
		bin_head.length_of_file_name ^= key_s;

		// (DWORD)idx ^ key_s
		int idx_len = bin_head.file_num * sizeof(struct IDX);
		pIDX  = (PBYTE)VirtualAlloc(NULL, idx_len, MEM_COMMIT, PAGE_READWRITE);
		pNAME = (PBYTE)VirtualAlloc(NULL, bin_head.length_of_file_name, MEM_COMMIT, PAGE_READWRITE);

		if (!pIDX || !pNAME)
		{
			if (pIDX) VirtualFree(pIDX, idx_len, MEM_DECOMMIT | MEM_FREE);
			if (pNAME) VirtualFree(pNAME, bin_head.length_of_file_name, MEM_DECOMMIT | MEM_FREE);
			CloseHandle(hFile);
			wsprintf(szBuffer, TEXT("内存分配错误, 跳过\r\n%s"), CurrentFile);
			AppendMsg(szBuffer);
			++dwNowProcess;
			continue;
		}

		ReadFile(hFile, pIDX, idx_len, &ByteRead, NULL);
		ReadFile(hFile, pNAME, bin_head.length_of_file_name, &ByteRead, NULL);

		// 解码 idx
		for (int i=0; i<idx_len; i+=4)
		{
			key_s ^= 0x65AC9365;
			key_c = ((key_s >> 1) ^ key_s) >> 3;
			key_d = ((key_s << 1) ^ key_s) << 3;
			key_s ^= (key_c ^ key_d);

			*((DWORD*)&pIDX[i]) ^= key_s;
		}
		//----------------保存一下idx
		HANDLE hSave = CreateFile(TEXT("idx"), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		WriteFile(hSave, pIDX, idx_len, &ByteRead, 0);
		CloseHandle(hSave);
		//------------------
		DWORD extract_file_num = 0;
		struct IDX * tmp;
		PBYTE content;
		char* NewFileName;

		for (int i=0; i<bin_head.file_num; ++i)
		{
			tmp = (struct IDX*)(pIDX+i*sizeof(struct IDX));
			NewFileName = (char*)((PBYTE)pNAME + tmp->file_name_offset);

			SetFilePointer(hFile, tmp->offset, 0, FILE_BEGIN);
			if (!(content = (PBYTE)VirtualAlloc(NULL, tmp->size+0x8, MEM_COMMIT, PAGE_READWRITE)))
			{
				int err = GetLastError();
				wsprintf(szBuffer, TEXT("无法保存%s - (1)%d内存分配错误%d"), NewFileName, tmp->size+0x8, err);
				AppendMsg(szBuffer);
				continue;
			}
			info.unknown = 1;
			info.hFile	 = hFile;
			info.offset  = tmp->offset;
			info.size	 = tmp->size;

			*((struct INFO**)content) = &info;
			*((DWORD*)content+0x4) = 0x0;

			ReadFile(hFile, content+0x8, tmp->size, &ByteRead, 0);		// 读取acp加密数据

			if (*((DWORD*)content+0x2) == 0x706361)		// "acp\0"
			{
				int raw_length;
				for (int j=0; j<4; ++j)			// acp中储存的源文件大小是大端数
					*((PBYTE)&raw_length+3-j) = *((PBYTE)content+0x8+0x4+j);
				PBYTE pRAW = (PBYTE)VirtualAlloc(NULL, raw_length, MEM_COMMIT, PAGE_READWRITE);
				if (!pRAW)
				{
					int err = GetLastError();
					VirtualFree(content, tmp->size, MEM_DECOMMIT);
					VirtualFree(content, 0, MEM_RELEASE);
					wsprintf(szBuffer, TEXT("无法保存%s - (2)%d内存分配错误%d"), NewFileName, raw_length, err);
					AppendMsg(szBuffer);
					continue;
				}
				
				DecoderACP(content, tmp->size, pRAW, raw_length);
				SplitFileNameAndSave(cur_dir, NewFileName, pRAW, raw_length);

				VirtualFree(pRAW, raw_length, MEM_DECOMMIT);
				VirtualFree(pRAW, 0, MEM_RELEASE);
			}
			else
				SplitFileNameAndSave(cur_dir, NewFileName, content, tmp->size);

			VirtualFree(content, tmp->size+0x8, MEM_DECOMMIT);
			VirtualFree(content, 0, MEM_RELEASE);
		}


		VirtualFree(pIDX, idx_len, MEM_DECOMMIT);
		VirtualFree(pIDX, 0, MEM_RELEASE);
		VirtualFree(pNAME, bin_head.length_of_file_name, MEM_DECOMMIT);
		VirtualFree(pNAME, 0, MEM_RELEASE);
		CloseHandle(hFile);
		++dwNowProcess;
	}
	ptp->file_num   = 0;
	ptp->thread_run = false;
	return 0;
}

PVOID pBlock;		// 71624C
DWORD global_1;		// 70D970 ebp计数
int global_2;		// 70D974
DWORD global_2_1[1000];	// 70D978			//  MDDDD 预留空间不够大导致解码越界问题
DWORD global_3;		// 716248

int MemAlloc()
{
	pBlock = GlobalLock(GlobalAlloc(GPTR, 0x669B4));
	if (pBlock)
		return 1;
	else
	{
		// ...Msg
		return 0;
	}
}

int MemFree()
{
	HGLOBAL hg;
	if (pBlock)
	{
		hg = GlobalHandle(pBlock);
		GlobalUnlock(hg);
		GlobalFree(hg);
		pBlock = 0;
	}
	return 1;
}

int FillBlock()
{
	PVOID p = pBlock;
	if (p)
	{
		for (int i=0; i<0x88CF; ++i, p = (PBYTE)p+0xC)
			*((DWORD*)p) = -1;
	}
	global_1 = 0x103;
	global_2 = 0x9;
	global_3 = 0x1FF;
	return 1;
}

int Process_1(PBYTE in, DWORD in_length, DWORD *poffset_of_in, DWORD *plast_result, DWORD* pdata, DWORD global_2)
{
	*pdata = 0;
	if (global_2 <=0 )
		return 1;

	int shift = global_2;

	while (*(poffset_of_in) < in_length)
	{
		shift -= *(plast_result);
		if (shift >= 0)
		{
			int d = *(in + 0x8 + *(poffset_of_in));
			d <<= shift;
			*pdata |= d;
			++(*(poffset_of_in));
			*(plast_result) = 0x8;

			if (shift > 0) continue;
			
			return 1;
		}
		else
		{
			int e = *(in+0x8+*(poffset_of_in));
			*(plast_result) = -shift;
			shift = (-shift) & 0xFF;
			e >>= shift;
			*pdata |= e;
			*(in+0x8+*(poffset_of_in)) &= (0xFF >> (8-shift));
			return 1;
		}
	}
}

int DecoderACP(PBYTE in, DWORD in_length, PBYTE out, int out_size)
{
	DWORD offset_of_in;			// 以"acp"为起始(即加密文件数据起始)来表示的偏移
	DWORD last_result;			// 上一次解码后更新的key(姑且这么说)
	DWORD last_data;			// 上一次解码得到的真实文件数据
	DWORD data;	// data:feb8	// 本次解码得到的真实文件数据
	DWORD data_feb7;			// 反正就这么个变量

	if (!in || !out || !out_size)
		return 0;

	if (!MemAlloc())
		return 0;

	PBYTE out_save = out;
	PBYTE out_end = out + out_size;

	FillBlock();

	offset_of_in = 0x8;		// in的前8字节数据是一个指针和一个双字0
	last_result  = 0x8;		// 初始为8
	if (!Process_1(in, in_length, &offset_of_in, &last_result, &last_data, global_2))		// 第一次结果放到last_data
	{
		MemFree();
		return 1;
	}
FLAG1:
	if (last_data == 0x100)
		goto RET_1;

	*(out_save++) = (BYTE)last_data;
	data_feb7 = last_data;

	if (out_save > out + out_size)
		goto RET_0;

	do
	{
		if (!Process_1(in, in_length, &offset_of_in, &last_result, &data, global_2))
			goto RET_1;

		if (data == 0x101)
		{
			++global_2;
			continue;
		}
		else if (data == 0x102)
		{
			FillBlock();
			if (Process_1(in, in_length, &offset_of_in, &last_result, &last_data, global_2))
				goto FLAG1;
			else
				goto RET_1;
		}
		else if (data == 0x100)
			goto RET_1;
		else
		{
			DWORD ecx = data;
			int tmp_cnt;	// eax
			if (ecx >= global_1)
			{// global_2_1;	70D978
				*((PBYTE)global_2_1) = (BYTE)data_feb7 & 0xFF;
				ecx = last_data;	// ebx
				tmp_cnt = 1;
			}
			else tmp_cnt = 0;

			while (ecx > 0xFF)
			{
				BYTE dl;
				ecx = 3 * ecx;
				dl = *((PBYTE)pBlock + ecx*4 + 8);
				*((PBYTE)global_2_1 + tmp_cnt) = dl;
				ecx = (DWORD)((DWORD*)pBlock + ecx);	// 临时当指针用
				ecx = *((DWORD*)ecx + 1);
				++tmp_cnt;
			}

			*((PBYTE)global_2_1 + tmp_cnt) = (BYTE)ecx;
			*(PBYTE)&data_feb7 = (BYTE)ecx;
			++tmp_cnt;
			if (out_save + tmp_cnt > out_end)
				goto RET_0;

			while (tmp_cnt > 0)
			{
				BYTE dl1 = *((PBYTE)global_2_1 - 1 + tmp_cnt);
				--tmp_cnt;
				*(((PBYTE)out_save)++) = dl1;
			}

			DWORD* eax = (DWORD*)pBlock + 3 * global_1;
			*(eax+1) = last_data;
			last_data = data;
			++global_1;
			*((PBYTE)eax+8) = (BYTE)ecx;
		}
	}while(1);


RET_0:
	MemFree();
	return 0;
RET_1:
	MemFree();
	return 1;
}