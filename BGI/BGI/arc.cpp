#include "arc.h"
#include "CBG_v2.h"

GET_IDX get_arc_idx[ARC_FILE_TYPE] = {get_arc_idx_0, get_arc_idx_1};

int Is_arc(HANDLE hFile)
{
	u8 buf[16];
	u32 r;

	SetFilePointer(hFile, 0, 0, FILE_BEGIN);
	ReadFile(hFile, buf, 12, &r, 0);

	for (int i=0; i<ARC_FILE_TYPE; ++i)
		if (!memcmp(buf, arc_file_type[i], magic_len[i]))
			return i;

	return ARC_FILE_TYPE;
}

int arc_extract_file_save(HANDLE hFile,const struct IDX * const idx, int file_num, u32 correct, wchar_t *cur_dir)
{
	CBG _cbg;
	bool succees_v2 = false;

	u8 *raw_data;
	u32 R, file_processsed = 0;

	wchar_t tmp[160];

	for (int i=0; i<file_num; ++i)
	{
		u8 *file = (u8*)VirtualAlloc(NULL, idx[i].size, MEM_COMMIT, PAGE_READWRITE);
		if (!file) return ERR_MEM;

		SetFilePointer(hFile, correct + idx[i].offset, 0, FILE_BEGIN);
		ReadFile(hFile, file, idx[i].size, &R, 0);

		int cbg_ver = 0;
		int raw_len = idx[i].size;
		raw_data = 0;
		if (Is_DSC(file))
		{
			if ((raw_len = DecodeDSC(&raw_data, file, idx[i].size)) < 0)
			{
				AppendMsg(TEXT("DSC解码错误"));
				VirtualFree(file, idx[i].size, MEM_DECOMMIT);
				VirtualFree(file, 0, MEM_RELEASE);
				continue;
			}
			if (Is_DSC_Image(raw_data, raw_len))
			{
				u32 bmp_len;
				u8 *bmp = Save_DSC_Bmp(raw_data, raw_len, &bmp_len);
				if (bmp)
				{
					u8 *tmp = raw_data;
					raw_data = bmp;
					free(tmp);
					raw_len = bmp_len;
					strcat((char*)idx[i].name, ".bmp");
				}
			}
		}
		else if (cbg_ver = Is_CBG(file))
		{
			if (cbg_ver == 1)
			{
				raw_len = DecodeCBG(&raw_data, file, idx[i].size);
				strcat((char*)idx[i].name, ".bmp");
				R = GetLastError();
			}
			else if (cbg_ver == 2)
			{
				succees_v2 = _cbg.Uncompress((s8*)file, idx[i].size);
			}
		}
		else
		{
			raw_data = (u8*)malloc(idx[i].size);
			memcpy(raw_data, file, idx[i].size);
		}
		// raw_data
		wchar_t wfilename[96];
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (PCSTR)idx[i].name, lstrlenA((PCSTR)idx[i].name), wfilename, 32);
		wfilename[lstrlenA((PCSTR)idx[i].name)] = '\0';
		if (cbg_ver == 2)
		{
			if (succees_v2)
			{
				wcscat(wfilename, L".bmp");
				if (!SplitFileNameAndSave(cur_dir, wfilename, _cbg.BmpData, _cbg.Size))
					++file_processsed;
			}
			else
			{
				wcscpy(tmp, wfilename);
				wcscat(tmp, L"解码失败！错误原因：");
				wcscat(tmp, _cbg.GetError());
				wcscat(tmp, L"\n");
				AppendMsg(tmp);
			}
			succees_v2 = false;
		}
		else if (!SplitFileNameAndSave(cur_dir, wfilename, raw_data, raw_len))
			++file_processsed;

		if (raw_data) free(raw_data);
		VirtualFree(file, idx[i].size, MEM_DECOMMIT);
		VirtualFree(file, 0, MEM_RELEASE);
	}
	return file_processsed;
}

int SplitFileNameAndSave(wchar_t *cur_dir, wchar_t *file_name, void* unpack, u32 file_length)
{
	DWORD ByteWrite;
	wchar_t buf[MAX_PATH] = {0}, buf2[MAX_PATH];

	wcscpy(buf, cur_dir);
	wcscat(buf, L"\\");
	wcscat(buf, file_name);

	int len = lstrlenW(buf);
	int i = wcslen(cur_dir) + 1;
	wchar_t *p = buf, *end = buf + len;
	while (p <= end && i < len)
	{
		while(buf[i] != '\\' && buf[i] != '/' && buf[i] != '\0') ++i;
	
		if (i<len)
		{
			wchar_t tmp = buf[i];
			buf[i] = '\0';

			CreateDirectory(p, 0);
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
			wsprintfW(buf2, L"[文件创建错误]%s", file_name);
			ret = ERR_FILE_CREATE;
			break;
		}

		WriteFile(hFile, unpack, file_length, &ByteWrite, NULL);

		if (ByteWrite != file_length)
		{
			wsprintfW(buf2, L"[文件写入错误]%s", file_name);
			ret = ERR_FILE_ERITE;
			break;
		}
		int t = GetLastError();
		if (!t || t == ERROR_ALREADY_EXISTS)
			wsprintfW(buf2, L"[已保存]%s", file_name);
		else
		{
			wsprintfW(buf2, L"[无法保存]%s,错误码%d", file_name, GetLastError());
			ret = ERR_FILE_OTHERS;
		}
	}while(0);

	AppendMsg(buf2);
	CloseHandle(hFile);
	return ret;
}

//*****************************************************************************
struct IDX* get_arc_idx_0(HANDLE hFile, u32 *file_num, u32 *idx_size)	// for "PackFile"
{
	u32 R;
	u8 *Zero[8];

	*file_num = 0;
	*idx_size = 0;
	SetFilePointer(hFile, 12, 0, FILE_BEGIN);
	ReadFile(hFile, file_num, 4, &R, 0);

	struct IDX *idx = (struct IDX *)malloc(sizeof(struct IDX) * *file_num);
	if (!idx) return 0; //ERR_MEM;
	
	for (int i=0; i<*file_num; ++i)
	{
		ReadFile(hFile,    idx[i].name, 16, &R, 0);
		ReadFile(hFile, &idx[i].offset,  4, &R, 0);
		ReadFile(hFile,   &idx[i].size,  4, &R, 0);
		ReadFile(hFile,           Zero,  8, &R, 0);
	}

	*idx_size = 0x20 * *file_num + 0x10;
	return idx;
}

struct IDX* get_arc_idx_1(HANDLE hFile, u32 *file_num, u32 *idx_size)	// for "BURIKO ARC20"
{
	u32 R;
	u8 *Zero[80];

	*file_num = 0;
	*idx_size = 0;
	SetFilePointer(hFile, 12, 0, FILE_BEGIN);
	ReadFile(hFile, file_num, 4, &R, 0);

	struct IDX *idx = (struct IDX *)malloc(sizeof(struct IDX) * *file_num);
	if (!idx) return 0; //ERR_MEM;

	for (int i=0; i<*file_num; ++i)
	{
		ReadFile(hFile,    idx[i].name, 96, &R, 0);
//		ReadFile(hFile,           Zero, 80, &R, 0);
		ReadFile(hFile, &idx[i].offset,  4, &R, 0);
		ReadFile(hFile,   &idx[i].size,  4, &R, 0);
		ReadFile(hFile,           Zero, 24, &R, 0);
	}

	*idx_size = 0x10 + 0x80 * *file_num;
	return idx;
}