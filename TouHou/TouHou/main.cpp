#include <Windows.h>
#include <CommDlg.h>
#include <strsafe.h>
#include "OffsetInfo.h"
#include "resource.h"

#define MAXPATH 350

HWND hEdit, hCombo;


typedef struct _THREADPARA
{
	wchar_t FileName[MAXPATH];
	DWORD GameIdx;
	int LoopTime;
}THREADPARA, *PTHREADPARA;



DWORD WINAPI Start(PVOID pv);
BOOL CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance,
					PSTR pCmdLine, int iCmdShow)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), 0, DlgProc, 0);
	return 0;
}

void AppendMsg(const wchar_t *szBuffer)
{
	static DWORD dwPos;
	if (0 == szBuffer)
	{
		dwPos = 0;
		SendMessage(hEdit, EM_SETSEL, 0, -1);
		SendMessage(hEdit, EM_REPLACESEL, FALSE, 0);
	} else {
		SendMessage(hEdit, EM_SETSEL, (WPARAM)&dwPos, (LPARAM)&dwPos);
		SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)szBuffer);
		SendMessage(hEdit, EM_GETSEL, 0, (LPARAM)&dwPos);
	}
	return;
}

BOOL CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static OPENFILENAME ofn;
	static THREADPARA tp;

	switch (msg)
	{
	case WM_INITDIALOG:
		SendMessage(hDlg, WM_SETICON, ICON_BIG,
					(LPARAM)LoadIcon((HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), MAKEINTRESOURCE(IDI_ICON1)));
		
		hEdit = GetDlgItem(hDlg, IDC_EDIT);
		SendMessage(hEdit, EM_LIMITTEXT, -1, 0);

		SetDlgItemText(hDlg, IDC_LOOP, L"1");

		hCombo = GetDlgItem(hDlg, IDC_GAMENAME);
		for (int i=0; i<sizeof(GameName)/sizeof(GameName[0]); ++i)
		{
			SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)GameName[i]);
		}

		memset(&ofn, 0, sizeof(ofn));
		ofn.lStructSize	= sizeof(ofn);
		ofn.hwndOwner	= hDlg;
		ofn.lpstrFilter	= L"所有文件(*.*)\0*.*\0\0";
		ofn.lpstrFile	= tp.FileName;
		ofn.nMaxFile	= MAXPATH;
		ofn.Flags		= OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
		
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			tp.LoopTime  = min(max(GetDlgItemInt(hDlg,  IDC_LOOP, 0, 0), 0), 15);
			if ((tp.GameIdx = SendMessage(hCombo, CB_GETCURSEL, 0, 0)) == CB_ERR)
			{
				MessageBox(hDlg, L"请选择对应游戏版本", L"提示", MB_ICONINFORMATION);
			}
			else if (GetOpenFileName(&ofn))
				CreateThread(0, 0, Start, &tp, 0, 0);
		}
		return TRUE;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	}
	return FALSE;
}

extern int Entrance(const wchar_t *CurDir, const wchar_t *PackName);

DWORD WINAPI Start(PVOID pv)
{
	wchar_t cur_dir[MAXPATH], szBuf[MAXPATH];
	LPTSTR CurrentFile;
	DWORD i, R, Saved = 0;
	PTHREADPARA ptp = (PTHREADPARA) pv;
	
	do {
		// 新建单独的目录
		StringCchCopy(cur_dir, MAXPATH, ptp->FileName);

		int l = lstrlen(cur_dir);
		while(l && cur_dir[l-1] != '\\') --l;
		cur_dir[l] = '\0';

		StringCchCat(cur_dir, MAXPATH, TEXT("[extract] "));
		StringCchCat(cur_dir, MAXPATH, &ptp->FileName[l]);
		CreateDirectory(cur_dir, 0);
		//--------------------------------------------------------
		Entrance(cur_dir, ptp->FileName);
		break;
		//--------------------------------------------------------
		HANDLE hFile = CreateFile(ptp->FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			AppendMsg(L"无法打开文件\r\n");
			break;
		}

		const PAIR **pp = &Index[ptp->GameIdx].pp;

		for (i=0; i<Index[ptp->GameIdx].bgm_num; ++i)
		{
			// 生成新文件名
			wchar_t NewName[MAXPATH];
			StringCchCopy(NewName, MAXPATH, cur_dir);
			StringCchCat(NewName, MAXPATH, L"\\");
			StringCchCat(NewName, MAXPATH, (*pp)[i].bgm_name);

			HANDLE hSave = CreateFile(NewName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
			if (hSave == INVALID_HANDLE_VALUE)
			{
				AppendMsg(L"无法保存文件\r\n");
				break;
			}

			// 分配缓冲区
			PBYTE buf = (PBYTE)VirtualAlloc(NULL, max((*pp)[i].head_len, (*pp)[i].loop_len), MEM_COMMIT, PAGE_READWRITE);
			if (!buf)
			{
				AppendMsg(L"内存不足\r\n");
				CloseHandle(hSave);
				break;
			}

			// 保存波形
			DWORD tmp;
			memcpy(buf, WAVEHEAD, sizeof(WAVEHEAD));
			*(PDWORD)(buf+4) = tmp = sizeof(WAVEHEAD) - 8 + (*pp)[i].head_len+(*pp)[i].loop_len*ptp->LoopTime;
			*(PDWORD)(buf+0x28) = tmp - sizeof(WAVEHEAD) + 8;
			WriteFile(hSave, buf, sizeof(WAVEHEAD), &R, 0);
			
			SetFilePointer(hFile, (*pp)[i].offset_head, 0, FILE_BEGIN);
			ReadFile(hFile, buf, (*pp)[i].head_len, &R, 0);
			WriteFile(hSave, buf, (*pp)[i].head_len, &R, 0);

			ReadFile(hFile, buf, (*pp)[i].loop_len, &R, 0);
			for (int k=0; k<ptp->LoopTime; ++k)
				WriteFile(hSave, buf, (*pp)[i].loop_len, &R, 0);
			
			CloseHandle(hSave);
			VirtualFree(buf, 0, MEM_RELEASE);
			StringCchPrintf(szBuf, MAXPATH, L"[已保存(%02d/%02d) %s\r\n", ++Saved, Index[ptp->GameIdx].bgm_num, (*pp)[i].bgm_name);
			AppendMsg(szBuf);
		}
		CloseHandle(hFile);
		if (Saved == Index[ptp->GameIdx].bgm_num) 
			AppendMsg(L"提取完成！\r\n");
		else
			AppendMsg(L"提取出错！\r\n");
	} while (0);

	return 0;
}