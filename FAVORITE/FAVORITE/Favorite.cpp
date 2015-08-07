#include <Windows.h>
#include "Favorite.h"

_UNCOM gfnUncompress;

int SeparateBmp::SetValue(DWORD h, DWORD w, DWORD b, PBYTE buf, DWORD cch)
{
	Height = h;
	Width  = w;
	Bpp    = b;
	FileSize = h * w * (b >> 3);
	FileNum  = cch / FileSize;
	FileSize += 54;	// 加上BMP文件头

	PBYTE p = buf, p_end = buf + cch;

	if (Data = (BYTE*)VirtualAlloc(NULL, cch, MEM_COMMIT, PAGE_READWRITE))
		memcpy(Data, buf, cch);
	else
		AppendMsg(L"SeparateBmp中内存分配失败！\r\n");
	return 0;
}

int SeparateBmp::SaveToFile(const wchar_t *dir, const wchar_t *NameWithoutSuffix)
{
	DWORD R;
	wchar_t format[MAXPATH] = {0};
	wchar_t newname[MAXPATH] = {0};
	wchar_t buf[MAXPATH] = {0};
	BYTE bmp[sizeof(BmpHeader)];
	wcscpy(format, dir);
	wcscat(format, L"\\");
	wcscat(format, NameWithoutSuffix);
	wcscat(format, L"_%03d.bmp");

	memcpy(bmp, BmpHeader, sizeof(BmpHeader));
	*(PDWORD)(bmp +  0x2) = FileSize;
	*(PDWORD)(bmp + 0x22) = FileSize - 54;
	*(PDWORD)(bmp + 0x12) = Width;
	*(PDWORD)(bmp + 0x16) = -Height;
	*(PBYTE) (bmp + 0x1C) = Bpp;

	// 把Alpha通道为透明(0x0)的像素换成白色
	PDWORD p = (PDWORD)(Data + 54);
	for (int i=0; i<((FileSize-54)>>2); ++i)
		if (!(p[i] & 0xff000000))
			p[i] = 0x00ffffff;

	DWORD FileSaved = 0;
	for (int i=0; i<FileNum; ++i)
	{
		wsprintf(newname, format, i);

		HANDLE hSave = CreateFile(newname, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (hSave == INVALID_HANDLE_VALUE)
		{
			wsprintf(buf, L"[创建失败] %s\r\n", newname);
			AppendMsg(buf);
			continue;
		}

		if (!WriteFile(hSave, bmp, sizeof(bmp), &R, 0) ||
			!WriteFile(hSave, Data+i*(FileSize-54), FileSize-54, &R, 0)
			)
		{
			wsprintf(buf, L"[写入失败] %s\r\n", newname);
			AppendMsg(buf);
		}
		else ++FileSaved;

		CloseHandle(hSave);

		wsprintf(buf, L"[已保存]%s\r\n", newname);
		AppendMsg(buf);
	}
	return FileSaved;
}

PackageInfo::PackageInfo(HANDLE hF):H_TotalFileNum(0), IdxOffset(0), NameOffset(0), IdxPtr(0), NamePtr(0)
{
	DWORD R;
	if (hF == INVALID_HANDLE_VALUE)
	{
		AppendMsg(L"无效的文件句柄！\r\n");
		return;
	}
	hFile = hF;
	SetFilePointer(hFile, 0, 0, FILE_BEGIN);
	ReadFile(hFile, &H_TotalFileNum, 4, &R, 0);

	IdxPtr  = IdxOffset  = sizeof(PACKAGEHEADER);
	NamePtr = NameOffset = sizeof(PACKAGEHEADER) + H_TotalFileNum * sizeof(PACKAGEINDEX);

	return;
}

int PackageInfo::GetNextIdx(PackageInfo::PPACKAGEINDEX pi, char *out, int cch)
{
	DWORD R;

	if (!pi || !out || !cch) return -1;
	
	SetFilePointer(hFile, IdxPtr, 0, FILE_BEGIN);
	ReadFile(hFile, pi, sizeof(PackageInfo::PACKAGEINDEX), &R, 0);

	SetFilePointer(hFile, NamePtr, 0, FILE_BEGIN);
	DWORD len = 0;
	do{
		ReadFile(hFile, &out[len++], 1, &R, 0);
	}while (len < cch && out[len-1] != 0);
	out[cch-1] = 0;

	IdxPtr  += sizeof(PackageInfo::PACKAGEINDEX);
	NamePtr += len;

	return 0;
}

int Exactehzc1File(PBYTE PackageData, PBYTE *OriginalData, DWORD PackageDataLen, SeparateBmp & sb) /*需要调用 gfnUncompress*/
{
	Phzc1HEADER Hzc1 = (Phzc1HEADER)PackageData;
	PNVSGHEADER Nvsg = (PNVSGHEADER)(PackageData + sizeof(hzc1HEADER));

	DWORD OriginalLen = Hzc1->OriginalFileLen;

	if (Nvsg->magic != 0x4753564E)	// "NVSG"
	{
		AppendMsg(L"NVSG标志符不匹配!\r\n");
		return -1;
	}

	if (!gfnUncompress) return 0;
	PBYTE RawData = (PBYTE)VirtualAlloc(NULL, OriginalLen + sizeof(BmpHeader), MEM_COMMIT, PAGE_READWRITE);
	gfnUncompress(RawData + sizeof(BmpHeader),
				  &OriginalLen,
				  PackageData + Hzc1->FileInfoLen + sizeof(hzc1HEADER),
				  PackageDataLen - Hzc1->FileInfoLen - sizeof(hzc1HEADER));

	MakeBmpFile(RawData, Hzc1->OriginalFileLen, Nvsg->BppType, Nvsg->Height, Nvsg->Width, sb);

	*OriginalData = RawData;
	return Hzc1->OriginalFileLen + 54;
}

int MakeBmpFile(PBYTE RawData, DWORD FileLen, DWORD BppType, DWORD Height, DWORD Width, SeparateBmp & sb)
{
	bool bNeedSeparate = false;
	BYTE Bpp;
	if (BppType == 0)		Bpp = 24;
	else if (BppType == 1)	Bpp = 32;
	else if (BppType == 2) {Bpp = 32;	bNeedSeparate = true; }
	else if (BppType == 3) {Bpp = 8;	AppendMsg(L"BppType 3\r\n"); }
	else
	{
		AppendMsg(L"未知颜色位数\r\n");
		return 0;
	}
	if (!bNeedSeparate)
	{
		memcpy(RawData, BmpHeader, sizeof(BmpHeader));

		*(PDWORD)(RawData + 0x2)  = FileLen + 54;
		*(PDWORD)(RawData + 0x22) = FileLen;
		*(PDWORD)(RawData + 0x12) = Width;
		*(PDWORD)(RawData + 0x16) = -Height;
		*(PBYTE)(RawData + 0x1C) = Bpp;

		PDWORD p = (PDWORD)(RawData + 54);
		for (int i=0; i<(FileLen>>2); ++i)	// 把Alpha通道为透明(0x0)的像素换成白色
			if (!(p[i] & 0xff000000))
				p[i] = 0x00ffffff;
	}
	else
	{
		sb.SetValue(Height, Width, Bpp, RawData+54, FileLen);	// 最初RawData里为Bmp头留出了空间
	}

	return 0;
}

int Enterence(wchar_t *PackageName, wchar_t *CurrentDir)
{
	wchar_t szBuf[MAX_PATH1];
	DWORD R, SavedFileNum = 0;

	if (!PackageName) return 0;

	HANDLE hFile = CreateFile(PackageName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		wsprintf(szBuf, L"无法打开文件%s, 错误码%d\r\n", PackageName, GetLastError());
		AppendMsg(szBuf);
		return 0;
	}

	PackageInfo PI(hFile);
	PackageInfo::PACKAGEINDEX pi;
	char AsciiName[MAX_PATH1];
	wchar_t UTFName[MAX_PATH1];

	for (int i=0; i<PI.FileNum(); ++i)
	{
		PI.GetNextIdx(&pi, AsciiName, MAX_PATH1-1);

		PBYTE PackData = (PBYTE)VirtualAlloc(NULL, pi.FileLength, MEM_COMMIT, PAGE_READWRITE);
		if (!PackData)
		{
			AppendMsg(L"内存无法分配，内存不够了么？\r\n");
			return 0;
		}
		
		SetFilePointer(hFile, pi.FileOffset, 0, FILE_BEGIN);
		ReadFile(hFile, PackData, pi.FileLength, &R, 0);

		// Code Page	932	shift_jis	ANSI/OEM Japanese; Japanese (Shift-JIS)
		MultiByteToWideChar(932, 0, AsciiName, -1, UTFName, MAX_PATH1-1);

		if (*(PDWORD)PackData == 0x5367674F || *(PDWORD)PackData == 0x46464952)	// "OggS" "RIFF"
		{
			if (*(PDWORD)PackData == 0x5367674F)
				wcscat(UTFName, L".ogg");
			else
				wcscat(UTFName, L".wav");

			if (!SplitFileNameAndSave(CurrentDir, UTFName, PackData, pi.FileLength))
				++SavedFileNum;
		}
		else if (*(PDWORD)PackData == 0x31637A68)	// "hzc1"
		{
			PBYTE OriginalData = 0;
			DWORD FileLen = 0;
			SeparateBmp sb;
			wcscat(UTFName, L".bmp");
			if (!(FileLen = Exactehzc1File(PackData, &OriginalData, pi.FileLength, sb)))
			{
				wsprintf(szBuf, L"文件解码失败！ - %s\r\n", UTFName);
				AppendMsg(szBuf);
			}
			else
			{
				if (sb.QueryFileNum())
				{
					UTFName[wcslen(UTFName)-4] = 0;	// 截掉.bmp后缀....
					SavedFileNum += !!sb.SaveToFile(CurrentDir, UTFName);
				}
				else if (!SplitFileNameAndSave(CurrentDir, UTFName, OriginalData, FileLen))
					++SavedFileNum;
			}
			VirtualFree(OriginalData, 0, MEM_RELEASE);
		}
		VirtualFree(PackData, 0, MEM_RELEASE);
	}
	if (PI.FileNum() == SavedFileNum)
	{
		wsprintf(szBuf, L"[提取完成(%d/%d)] %s\r\n", SavedFileNum, SavedFileNum, PackageName);
		AppendMsg(szBuf);
	}
	else
	{
		wsprintf(szBuf, L"[提取完成(%d/%d)] %s\r\n有%d个文件出错",
					SavedFileNum, PI.FileNum(), PackageName, PI.FileNum()-SavedFileNum);
		MessageBox(0, szBuf, L"提示", MB_ICONWARNING);
	}
	return 0;
}

int SplitFileNameAndSave(const wchar_t *cur_dir, const wchar_t *file_name, void* unpack, unsigned long file_length)
{
	DWORD ByteWrite;
	wchar_t buf[MAX_PATH] = {0}, buf2[MAX_PATH];

	lstrcpyW(buf, cur_dir);
	lstrcatW(buf, L"\\");
	lstrcatW(buf, file_name);

	int len = lstrlenW(buf);
	int i = lstrlenW(cur_dir) + 1;
	wchar_t *p = buf, *end = buf + len;
	while (p <= end && i < len)
	{
		while(buf[i] != '\\' && buf[i] != '/' && buf[i] != '\0') ++i;
	
		if (i<len)
		{
			wchar_t tmp = buf[i];
			buf[i] = '\0';

			CreateDirectoryW(p, 0);
			buf[i] = tmp;
			++i;
			p = buf + i;
		}
	}

	HANDLE hFile;
	int ret = 0;
	do{
		hFile = CreateFile(p, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			wsprintfW(buf2, L"[文件创建错误]%s\r\n", p);
			ret = -1;
			break;
		}

		WriteFile(hFile, unpack, file_length, &ByteWrite, NULL);

		if (ByteWrite != file_length)
		{
			wsprintfW(buf2, L"[文件写入错误]%s\r\n", p);
			ret = -2;
			break;
		}
		int t = GetLastError();
		if (!t || t == ERROR_ALREADY_EXISTS)
			wsprintfW(buf2, L"[已保存]%s\r\n", p);
		else
		{
			wsprintfW(buf2, L"[无法保存]%s,错误码%d\r\n", p, GetLastError());
			ret = -3;
		}
	}while(0);

	AppendMsg(buf2);
	CloseHandle(hFile);
	return ret;
}