#include <Windows.h>
#include <vector>
#include <string>
#include <strsafe.h>

#include"resource.h"

using std::vector;
using std::wstring;

HWND hList, hText;
HANDLE hEvent;
const wchar_t FileName [][MAX_PATH] = {
/*	L"0+x2y0.bmp",
	L"1+x4y10.bmp",
	L"2+x6y20.bmp",
	L"3+x8y30.bmp",
	L"4+x0y40.bmp",

	L"0+x0y0.bmp",
	L"1+x0y88.bmp",
	L"2+x0y176.bmp",
	L"3+x0y264.bmp",
	L"4+x0y360.bmp",
	L"5+x0y448.bmp",
	L"6+x0y536.bmp",
	L"7+x0y624.bmp",

	L"アリス_小_ナース2_10困り01_00+x144y0.bmp",
	L"アリス_小_ナース2_10困り01_01+x96y24.bmp",
	L"アリス_小_ナース2_10困り01_02+x88y56.bmp",
	L"アリス_小_ナース2_10困り01_03+x80y88.bmp",
	L"アリス_小_ナース2_10困り01_04+x80y120.bmp",
	L"アリス_小_ナース2_10困り01_05+x80y152.bmp",
	L"アリス_小_ナース2_10困り01_06+x72y184.bmp",
	L"アリス_小_ナース2_10困り01_07+x56y216.bmp",*/

	L"アリス_小_私服1_20怒り02_01+x584y88.bmp",
	L"アリス_小_私服1_20怒り02_02+x544y176.bmp",
	L"アリス_小_私服1_20怒り02_03+x536y264.bmp",
	L"アリス_小_私服1_20怒り02_04+x424y360.bmp",
	L"アリス_小_私服1_20怒り02_05+x432y448.bmp",
	L"アリス_小_私服1_20怒り02_06+x376y536.bmp",
	L"アリス_小_私服1_20怒り02_07+x336y624.bmp",
};

class BmpFileGroup
{
	enum {
		BMPHEADERLEN   = 54,
		ERR_FILEOPEN   = -1,
		ERR_MEM        = -2,
		ERR_FILECREATE = -3,
		ERR_FILEFORMAT = -4,
	};

	typedef struct
	{
		unsigned long  Offset_x;
		unsigned long  Offset_y;
		unsigned long  Width;
		unsigned long  Height;
		unsigned long  FileLen;
		unsigned char *FileData;
		wstring FileName;
	} FILEINFO;
	long          m_Error;
	unsigned long m_Width;		// 一组中最大的宽度
	unsigned long m_Height;		// 一组中最大的高度
	unsigned long m_Bpp;
	vector<FILEINFO> FileInfo;

	BmpFileGroup(BmpFileGroup &) {};
public:
	BmpFileGroup():m_Width(0), m_Height(0), m_Bpp(0), m_Error(0) {}
	bool Init(const wchar_t FileName[][MAX_PATH], unsigned long Num);
	void Reset()
	{
		for (int i=FileInfo.size()-1; i>=0; --i)
			VirtualFree(FileInfo[i].FileData, 0, MEM_RELEASE);
		FileInfo.clear();
		m_Width = m_Height = m_Bpp = m_Error = 0;
	}
	void SetErrorNo(long e) {m_Error = e;}
	long GetErrorNo(void) {return m_Error;}
	~BmpFileGroup() {};

	bool Merge(void);
};


bool BmpFileGroup::Init(const wchar_t FileName[][MAX_PATH], unsigned long Num)
{
	unsigned long R;
	FILEINFO      fi;
	BYTE          PicInfo[BMPHEADERLEN], *Tmp;

	m_Width = m_Height = 0;
	m_Bpp = 32;

	for (unsigned long i=0; i<Num; ++i)
	{
		HANDLE hFile = CreateFile(FileName[i], GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			SetErrorNo(ERR_FILEOPEN);
			return false;
		}

		fi.FileLen = GetFileSize(hFile, 0) - BMPHEADERLEN;

		if (!(fi.FileData = (unsigned char *) VirtualAlloc(NULL, fi.FileLen, MEM_COMMIT, PAGE_READWRITE)))
		{
			SetErrorNo(ERR_MEM);
			CloseHandle(hFile);
			return false;
		}
		
		// 提取图片信息
		ReadFile(hFile, PicInfo, BMPHEADERLEN, &R, 0);

		fi.Width  = *(unsigned long *)&PicInfo[0x12];
		fi.Height = *(unsigned long *)&PicInfo[0x16];


		fi.FileName = FileName[i];

		unsigned long k = fi.FileName.size();
		while (k>0 && fi.FileName[k-1] != 'y') --k;
		swscanf_s(&FileName[i][k], L"%u", &fi.Offset_y);
		while (k>0 && fi.FileName[k-1] != 'x') --k;
		swscanf_s(&FileName[i][k], L"%u", &fi.Offset_x);


		if (*(unsigned long *)&PicInfo[0x12] + fi.Offset_x > m_Width)
			m_Width = *(unsigned long *)&PicInfo[0x12] + fi.Offset_x;

		if (*(unsigned long *)&PicInfo[0x16] + fi.Offset_y > m_Height)
			m_Height = *(unsigned long *)&PicInfo[0x16] + fi.Offset_y;


		// 去文件头
		ReadFile(hFile, fi.FileData, fi.FileLen, &R, 0);
		DWORD back = fi.FileLen;
		if (PicInfo[0x1c] == 24)
		{
			fi.FileLen = fi.FileLen / 3 * 4;

			if (!(Tmp = (unsigned char *) VirtualAlloc(NULL, fi.Height*fi.Width*4, MEM_COMMIT, PAGE_READWRITE)))
			{
				SetErrorNo(ERR_MEM);
				CloseHandle(hFile);
				return false;
			}
			PBYTE p = fi.FileData;
			for (int row=0; row<fi.Height; ++row)
			{
				p = fi.Width*3*row + fi.FileData;
				int remain = fi.Width*3;
				unsigned long n = fi.Width*4*row;
				for (; remain>0; n+=4)
				{
					Tmp[n]   = *p++;
					Tmp[n+1] = *p++;
					Tmp[n+2] = *p++;
					Tmp[n+3] = 0xff;
					remain -= 3;
				}
			}

			VirtualFree(fi.FileData, 0, MEM_RELEASE);
			fi.FileData = Tmp;

			
		}
/*			SetErrorNo(ERR_FILEFORMAT);
			CloseHandle(hFile);
			VirtualFree(fi.FileData, 0, MEM_RELEASE);
			fi.FileData = 0;
			return false;
		}
*/

		
		CloseHandle(hFile);

		

		FileInfo.push_back(fi);
	}

	

	return true;
}


const unsigned char BmpHeader [54] = {
	0x42, 0x4D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x20, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


bool BmpFileGroup::Merge(void)
{
	unsigned long BytesPerPixel = m_Bpp / 8;
	unsigned long BytesPerRow   = (m_Width * BytesPerPixel + BytesPerPixel-1) & ~(BytesPerPixel-1);
	unsigned long NewFileLen    = BMPHEADERLEN + m_Height * BytesPerRow;
	unsigned char *pNewFile     = (unsigned char *)VirtualAlloc(NULL, NewFileLen, MEM_COMMIT, PAGE_READWRITE);
	if (!pNewFile)
	{
		SetErrorNo(ERR_MEM);
		return false;
	}

	if (m_Bpp == 32)
		memset(pNewFile, 0, NewFileLen);


	unsigned char *pOrigin = pNewFile + BMPHEADERLEN;
	unsigned char *pEnd    = pNewFile + BMPHEADERLEN + m_Height * BytesPerRow;
	unsigned char *p       = pEnd;

	for (unsigned long i=0; i<FileInfo.size(); ++i)
	{
		unsigned long x = FileInfo[i].Offset_x;
		unsigned long y = FileInfo[i].Offset_y;
		unsigned long o_BytesPerRow = (FileInfo[i].Width * BytesPerPixel + BytesPerPixel-1) & ~(BytesPerPixel-1);

		if (x > m_Width || y > m_Height)
		{
			MessageBox(0, L"建议检查是否有不符合规格的文件", 0, 0);
			SetErrorNo(ERR_FILEFORMAT);
			break;
		}
		p = pEnd - (y+1) * BytesPerRow + x * BytesPerPixel;

		/////////////////////////////////////////////
		/*////  Debug

		for (DWORD g=0; g<FileInfo.size(); ++g)
		{
			HANDLE h = CreateFile(FileInfo[g].FileName.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			unsigned long R;
			WriteFile(h, FileInfo[g].FileData, FileInfo[g].FileLen, &R, 0);
			CloseHandle(h);
		}

		*/////////////////////////////////////////////
		for (int k=FileInfo[i].Height-1; p>=pOrigin && k>=0; --k)
		{
			memcpy(p, FileInfo[i].FileData + k * o_BytesPerRow, o_BytesPerRow);
			p -= BytesPerRow;
		}
	}


	if (m_Bpp == 32)
	{
		// Alpha混合
		PBYTE p = pOrigin;
		for (DWORD i = 0; i < m_Width * m_Height; ++i)
		{
			p[0] = p[0] * p[3] / 255 + 255 - p[3];
			p[1] = p[1] * p[3] / 255 + 255 - p[3];
			p[2] = p[2] * p[3] / 255 + 255 - p[3];
			p += 4;
		}
	}


	memcpy(pNewFile, BmpHeader, BMPHEADERLEN);
	*(unsigned long *)(pNewFile + 0x2) = NewFileLen;
	*(unsigned long *)(pNewFile + 0x12) = m_Width;
	*(unsigned long *)(pNewFile + 0x16) = m_Height;
	                 *(pNewFile + 0x1c) = (unsigned char)m_Bpp;
	*(unsigned long *)(pNewFile + 0x22) = NewFileLen - BMPHEADERLEN;

	int pos = wcslen(FileInfo[0].FileName.c_str()), twoplus = 2;
	for (; pos>0 && twoplus; --pos)
		if (FileInfo[0].FileName[pos-1] == '+')
			--twoplus;

	if (pos > 0)
	{
		FileInfo[0].FileName = FileInfo[0].FileName.substr(0, pos);//FileInfo[0].FileName[pos] = '\0';
		FileInfo[0].FileName += wstring(L".bmp");
	}

	HANDLE hSave = CreateFile(FileInfo[0].FileName.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hSave == INVALID_HANDLE_VALUE)
	{
		SetErrorNo(ERR_FILECREATE);
		VirtualFree(pNewFile, 0, MEM_RELEASE);
		return false;
	}

	unsigned long R;
	WriteFile(hSave, pNewFile, NewFileLen, &R, 0);

	CloseHandle(hSave);

	VirtualFree(pNewFile, 0, MEM_RELEASE);
	return true;

}


BOOL CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI Thread(PVOID pv);



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR pCmdLine, int iCmdShow)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), 0, DlgProc, 0);
	return 0;
}


void OnDropFiles(HDROP hDrop, HWND hDlg, wchar_t *DirName, DWORD Len)
{
	HANDLE			hFindFile;
	WIN32_FIND_DATA	fd;
	wchar_t         MsgBuf[16], Search[MAX_PATH], Find[MAX_PATH];
	


	DragQueryFile(hDrop, 0, DirName, Len);
	if (!(GetFileAttributes(DirName) & FILE_ATTRIBUTE_DIRECTORY))
		return;

	
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
				SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)Find);
				StringCchPrintf(MsgBuf, 16, L"%u", cnt++);
				SetDlgItemText(hDlg, IDC_TEXT, MsgBuf);
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
//		SendMessage(hDlg, WM_SETICON, ICON_BIG,
//					(LPARAM)LoadIcon((HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), MAKEINTRESOURCE(IDI_ICON1)));
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
	
//			if (!wcsncmp(FileName[i], L"aka109_a1_03_06m2", 17))
//				i = i;

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
				MessageBox(0, L"@@@", 0, 0);	// 错误信息

			bf.Reset();
		}

		MessageBox(0, L"转换结束", 0, 0);
		ResetEvent(hEvent);

	}
	return 0;
}