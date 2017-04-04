#include <Windows.h>
#include <strsafe.h>
#include "BmpFileGroup.h"
#include "resource.h"


HWND hList, hText;
HANDLE hEvent;


BOOL CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI Thread(PVOID pv);



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR pCmdLine, int iCmdShow)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), 0, DlgProc, 0);
	return 0;
}


bool IsFileExtensionBmp(const wchar_t *Name)
{
	int k = wcslen(Name);
	while (k>0 && Name[k-1] != '.') --k;
	return !wcscmp(&Name[k], L"bmp");
}


void OnDropFiles(HDROP hDrop, HWND hDlg, wchar_t *DirName, DWORD Len)
{
	HANDLE			hFindFile;
	WIN32_FIND_DATA	fd;
	wchar_t         MsgBuf[16], Search[MAX_PATH], Find[MAX_PATH];
	


	DragQueryFile(hDrop, 0, DirName, Len);
	if (!(GetFileAttributes(DirName) & FILE_ATTRIBUTE_DIRECTORY))
	{
		MessageBox(hDlg, L"请拖放一个文件夹", L"提示", MB_ICONINFORMATION);
		return;
	}

	
	StringCchCopy(Search, MAX_PATH, DirName);
	StringCchCat(Search, MAX_PATH, L"\\*.*");
	
	DWORD cnt = 1;
	if (INVALID_HANDLE_VALUE != (hFindFile = FindFirstFile(Search, &fd)))
	{
		do
		{
			// 完整文件名
			StringCchCopy(Find, MAX_PATH, DirName);
			StringCchCat(Find, MAX_PATH, L"\\");
			StringCchCat(Find, MAX_PATH, fd.cFileName);

			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				if (IsFileExtensionBmp(Find))
				{
					SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)Find);

					StringCchPrintf(MsgBuf, 16, L"%u", cnt++);
					SetDlgItemText(hDlg, IDC_TEXT, MsgBuf);
				}
			}

		}while(FindNextFile(hFindFile, &fd));
		FindClose(hFindFile);
	}
	

	DragFinish(hDrop);

	return;
}


static bool bExit;


BOOL CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	
	static wchar_t CurDir[MAX_PATH];

	switch (msg)
	{
	case WM_INITDIALOG:
		SendMessage(hDlg, WM_SETICON, ICON_BIG,
					(LPARAM)LoadIcon((HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), MAKEINTRESOURCE(IDI_ICON1)));
		bExit = false;

		hList = GetDlgItem(hDlg, IDC_LIST);
		hText = GetDlgItem(hDlg, IDC_TEXT);

		hEvent = CreateEvent(0, TRUE, FALSE, 0);
		CreateThread(0, 0, Thread, CurDir, 0, 0);

		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			SetEvent(hEvent);
			return TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			while (SendMessage(hList, LB_DELETESTRING, 0, 0));
			return TRUE;
		}
		return FALSE;

	case WM_DROPFILES:
		OnDropFiles((HDROP)wParam, hDlg, CurDir, MAX_PATH);
		return TRUE;

	case WM_CLOSE:
		bExit = true;
		SetEvent(hEvent);
		Sleep(200);

		EndDialog(hDlg, 0);
		return TRUE;
	}
	return FALSE;
}

DWORD WINAPI Thread(PVOID pv)
{
	wchar_t		*CurDir = (wchar_t *)pv;
	wchar_t		 FileName[64][MAX_PATH];
	DWORD		 dwNowProcess = 0;
	BmpFileGroup bf;
	
	while (1)
	{
		WaitForSingleObject(hEvent, INFINITE);

		if (bExit) break;

		
		while (SendMessage(hList, LB_GETCOUNT, 0, 0))
		{
			DWORD i=0;
			SendMessage(hList, LB_GETTEXT, 0, (LPARAM)FileName[0]);

			

			int k = wcslen(FileName[i]), m = k;
			while (m>0 && FileName[i][m-1] != '\\') --m;
			for (; m<k && FileName[i][m]!='+'; ++m);


			SendMessage(hList, LB_DELETESTRING, 0, 0);
			while (i<64 && SendMessage(hList, LB_GETTEXT, 0, (LPARAM)FileName[i+1]))
			{
				int k1 = wcslen(FileName[i+1]), m1 = k1;
				while (m1>0 && FileName[i+1][m1-1] != '\\') --m1;
				for (; m1<k && FileName[i+1][m1]!='+'; ++m1);

				if (m1 == m && !wcsncmp(FileName[i], FileName[i+1], m))
				{
					++i;
					SendMessage(hList, LB_DELETESTRING, 0, 0);
				} else
					break;
			}

			bf.Init(FileName, i+1);
			
			if (!bf.Merge())
			{
				switch(bf.GetErrorNo())
				{
				case BmpFileGroup::ERR_FILECREATE:
					MessageBox(0, L"无法创建新文件", L"发生错误", MB_ICONWARNING);
					break;

				case BmpFileGroup::ERR_FILEFORMAT:
					MessageBox(0, L"文件格式错误", L"发生错误", MB_ICONWARNING);
					break;

				case BmpFileGroup::ERR_FILEOPEN:
					MessageBox(0, L"无法打开文件", L"发生错误", MB_ICONWARNING);
					break;

				case BmpFileGroup::ERR_MEM:
					MessageBox(0, L"内存不足", L"发生错误", MB_ICONWARNING);
					break;
				}
			}
			bf.Reset();
		}

		MessageBox(0, L"转换结束", L"完成", MB_ICONINFORMATION);
		ResetEvent(hEvent);

	}
	return 0;
}