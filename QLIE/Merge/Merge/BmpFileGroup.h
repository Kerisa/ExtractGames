
#pragma once

#include <Windows.h>

#include <string>
#include <vector>
using std::wstring;
using std::vector;


class BmpFileGroup
{
public:
	enum {
		MAXPATH       = 260,
		BMPHEADERLEN   = 54,
		ERR_FILEOPEN   = -1,
		ERR_MEM        = -2,
		ERR_FILECREATE = -3,
		ERR_FILEFORMAT = -4,
	};

private:
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
	bool Init(const wchar_t FileName[][MAXPATH], unsigned long Num);
	void Reset()
	{
		for (int i=FileInfo.size()-1; i>=0; --i)
			VirtualFree(FileInfo[i].FileData, 0, MEM_RELEASE);
		FileInfo.clear();
		m_Width = m_Height = m_Bpp = m_Error = 0;
	}
	void SetErrorNo(long e) {m_Error = e;}
	long GetErrorNo(void) {return m_Error;}
	~BmpFileGroup() {Reset();}

	bool Merge(void);
};


bool BmpFileGroup::Init(const wchar_t FileName[][MAXPATH], unsigned long Num)
{
	unsigned long R;
	FILEINFO      fi;
	unsigned char PicInfo[BMPHEADERLEN], *Tmp;

	m_Width = m_Height = 0;
	m_Bpp = 32;

	for (unsigned long i=0; i<Num; ++i)
	{
		fi.FileName = FileName[i];

		unsigned long k = fi.FileName.size();
		while (k>0 && fi.FileName[k-1] != 'y') --k;
		swscanf_s(&FileName[i][k], L"%u", &fi.Offset_y);
		while (k>0 && fi.FileName[k-1] != 'x') --k;
		swscanf_s(&FileName[i][k], L"%u", &fi.Offset_x);


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

		
		if (*(unsigned long *)&PicInfo[0x12] + fi.Offset_x > m_Width)
			m_Width = *(unsigned long *)&PicInfo[0x12] + fi.Offset_x;

		if (*(unsigned long *)&PicInfo[0x16] + fi.Offset_y > m_Height)
			m_Height = *(unsigned long *)&PicInfo[0x16] + fi.Offset_y;


		// 去文件头
		ReadFile(hFile, fi.FileData, fi.FileLen, &R, 0);
		unsigned long back = fi.FileLen;
		if (PicInfo[0x1c] == 24)
		{	// 调整为32位bmp
			fi.FileLen = fi.FileLen / 3 * 4;

			if (!(Tmp = (unsigned char *) VirtualAlloc(NULL, fi.Height*fi.Width*4, MEM_COMMIT, PAGE_READWRITE)))
			{
				SetErrorNo(ERR_MEM);
				CloseHandle(hFile);
				return false;
			}
			unsigned char *p = fi.FileData;
			for (int row=0; row<fi.Height; ++row)
			{
				p = fi.Width*3*row + fi.FileData;
				int remain = fi.Width;
				unsigned long n = fi.Width*4*row;
				for (; remain>0; n+=4, --remain)
				{
					Tmp[n]   = *p++;
					Tmp[n+1] = *p++;
					Tmp[n+2] = *p++;
					Tmp[n+3] = 0xff;
				}
			}

			VirtualFree(fi.FileData, 0, MEM_RELEASE);
			fi.FileData = Tmp;

			
		}

		
		CloseHandle(hFile);

		FileInfo.push_back(fi);
	}	

	return true;
}


static const unsigned char BmpHeader [54] = {
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
			return false;
		}
		p = pEnd - (y+1) * BytesPerRow + x * BytesPerPixel;

		
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
		FileInfo[0].FileName = FileInfo[0].FileName.substr(0, pos);
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