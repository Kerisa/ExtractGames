#include "xp3.h"

int is_xp3_file(HANDLE hFile)
{
	u32 R;
	char magic[11];

	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	ReadFile(hFile, magic, 11, &R, NULL);
	return !memcmp(magic, "XP3\r\n \n\x1A\x8B\x67\x01", 11);
}

u8* uncompress_xp3_idx(HANDLE hFile, u32 *idx_len, UNCOM unCom)
{
	u32 ByteRead;
	xp3_file_header header;

	SetFilePointer(hFile, 11, NULL, FILE_BEGIN);
	ReadFile(hFile, &header.offset_lo, 4, &ByteRead, NULL);
	ReadFile(hFile, &header.offset_hi, 4, &ByteRead, NULL);

	if (header.offset_lo != 0x17)
		SetFilePointer(hFile, header.offset_lo, (PLONG)&header.offset_hi, FILE_BEGIN);
	else
	{
		ReadFile(hFile, &header.minor_version, 4, &ByteRead, NULL);
		ReadFile(hFile, &header.flag, 1, &ByteRead, NULL);
		ReadFile(hFile, &header.index_size_lo, 4, &ByteRead, NULL);
		ReadFile(hFile, &header.index_size_hi, 4, &ByteRead, NULL);
		ReadFile(hFile, &header.index_offset_lo, 4, &ByteRead, NULL);
		ReadFile(hFile, &header.index_offset_hi, 4, &ByteRead, NULL);

		SetFilePointer(hFile, header.index_offset_lo, (PLONG)&header.index_offset_hi, FILE_BEGIN);
	}

	u8  idx_flag;
	u32 idx_size_lo;
	u32 idx_size_hi;
	u32 idx_uncom_lo;
	u32 idx_uncom_hi;

	ReadFile(hFile,     &idx_flag, 1, &ByteRead, NULL);
	ReadFile(hFile,  &idx_size_lo, 4, &ByteRead, NULL);
	ReadFile(hFile,  &idx_size_hi, 4, &ByteRead, NULL);
	if (idx_flag)
	{
		ReadFile(hFile, &idx_uncom_lo, 4, &ByteRead, NULL);
		ReadFile(hFile, &idx_uncom_hi, 4, &ByteRead, NULL);
	}
	else
	{
		idx_uncom_lo = idx_size_lo;
		idx_uncom_hi = idx_size_hi;
	}
	
	u8 * idx	 = (u8*)VirtualAlloc(NULL,  idx_size_lo, MEM_COMMIT, PAGE_READWRITE);
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
	VirtualFree(idx, idx_size_lo, MEM_DECOMMIT);
	VirtualFree(idx, 0, MEM_RELEASE);

	*idx_len = idx_uncom_lo;
	return idx_raw;
}

static u8* get_file_thunk(u8 *pointer, struct file_entry *fe, u32 *split_file, u8 *idx_end)
{
	static const u32 _file = 0x656C6946;
	static const u32 _adlr = 0x726c6461;
	static const u32 _segm = 0x6d676573;
	static const u32 _info = 0x6f666e69;

	if (*((u32*)pointer) != _file)		// 都是以"File"块开始
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

	while (pointer < first_end)
	{
		switch (*(u32*)pointer)
		{
		default:
			++pointer;
			break;

		case _adlr:
			pointer += 0xC;
			fe->crc = *((u32*)pointer);
			pointer += 4;
			break;

		case _segm:
			if (*(u32*)(pointer+4) % 0x1c == 0)
			{
				*split_file = *(u32*)(pointer+4) / 0x1c;
				pointer += 0xC;
				for (int i=0; i<*split_file; ++i)
				{
					fe[i].compress_flag = *(u32*)pointer;   	pointer += 4;	// 1 compressed
					fe[i].offset		= *(u64*)pointer;		pointer += 8;
					fe[i].orig_length   = *(u64*)pointer;		pointer += 8;
					fe[i].pkg_length    = *(u64*)pointer;		pointer += 8;
				}
			}
			else
			{
				AppendMsg(TEXT("错误的文件索引"));
				*split_file = 0;
				while(*(u32*)pointer != _file) ++pointer;	// 跳过这个索引
			}
			break;

		case _info:
			pointer += 0xc;
			fe->encryption_flag = *((u32*)pointer);	// 好像这个标志也没啥用
			pointer += 0x14;

			int buf_size = (int)*((u16*)pointer);
			if(buf_size >= sizeof(fe->file_name)/sizeof(fe->file_name[0]))
			{
				MessageBox(0, TEXT("文件名超出缓冲区长度"), TEXT("提示"), MB_ICONWARNING | MB_OK);
				buf_size = sizeof(fe->file_name)/sizeof(fe->file_name[0]) - 1;
			}
			pointer += 0x2;
			lstrcpyW(fe->file_name, (wchar_t*)pointer);
			fe->file_name[buf_size] = '\0';
			break;
		}
	}
	return pointer;
}

static int SplitFileNameAndSave(wchar_t *cur_dir, wchar_t *file_name, void* unpack, u32 file_length)
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
		if (buf[i] == '/') buf[i] = '\\';
		if (i<len)
		{
			wchar_t tmp = buf[i];
			buf[i] = '\0';

			CreateDirectoryW(p, 0);
			buf[i] = tmp;
			++i;
		}
	}

	HANDLE hFile;
	int ret = 0;
	do{
		hFile = CreateFile(buf, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			wsprintfW(buf2, L"[文件创建错误]%s", p);
			ret = ERR_FILE_CREATE;
			break;
		}

		WriteFile(hFile, unpack, file_length, &ByteWrite, NULL);

		if (ByteWrite != file_length)
		{
			wsprintfW(buf2, L"[文件写入错误]%s", p);
			ret = ERR_FILE_ERITE;
			break;
		}
		
		int t = GetLastError();
		if (!t || t == ERROR_ALREADY_EXISTS)
			wsprintfW(buf2, L"[已保存]%s", p);
		else
		{
			wsprintfW(buf2, L"[无法保存]%s", p);
			ret = ERR_FILE_OTHERS;
		}
	}while(0);

	AppendMsg(buf2);
	CloseHandle(hFile);
	return ret;
}

int xp3_extract_file_save(const HANDLE hFile, u8 *xp3_idx, int idx_len, u32 *file_num, char *game, UNCOM unCom, wchar_t *cur_dir)
{
	_XOR_DECODE p_decode = (_XOR_DECODE)0x80000000;
	struct file_entry fe[20];	// 目前看到的最多分成5段(segm长度0x8c)，没个准头
	u8 *p, *idx_end;
	u32 R, split_file, game_idx, offset_hi, cnt_savefile = 0;

	p		= xp3_idx;
	idx_end = xp3_idx + idx_len;
	
	p += 0x4;
	p += *(u32*)p + 0x8;	// skip the protection warning
	
	if (!strcmp(game, unencry_game))	// 决定解密使用的函数
		p_decode = 0;
	else for (int i=0; i<sizeof(simple_xor_game)/sizeof(simple_xor_game[0]); ++i)
		if (!strcmp(game, simple_xor_game[i].name))
		{
			p_decode = simple_xor_game[i].p_decode;
			game_idx = i;
			break;
		}

	while(p < idx_end)
	{
		p = get_file_thunk(p, fe, &split_file, idx_end);
		if (!p) break;
		if (split_file) ++(*file_num);

		if (split_file > sizeof(fe)/sizeof(fe[0]))
		{
			MessageBox(0, L"文件分段数量过多，提取中止！", 0, MB_ICONERROR);
			break;
		}

		u32 file_pkg_len = 0;
		u32 file_org_len = 0;
		for (int i=0; i<split_file; ++i)
		{
			file_pkg_len += fe[i].pkg_length;
			file_org_len += fe[i].orig_length;
		}

		u32 file_read = 0;
		u8 *cipher = (u8*)VirtualAlloc(NULL, file_pkg_len, MEM_COMMIT, PAGE_READWRITE);

		for (int i=0; i<split_file; ++i)
		{
			offset_hi = (u32)(fe[i].offset >> 32);
			SetFilePointer(hFile, (u32)fe[i].offset, (PLONG)&offset_hi, FILE_BEGIN);
			ReadFile(hFile, cipher+file_read, (u32)fe[i].pkg_length, &R, NULL);
			file_read += fe[i].pkg_length;
		}

		u8* unpack		  = (u8*) VirtualAlloc(NULL, file_org_len, MEM_COMMIT, PAGE_READWRITE);
		u32 unpack_len	  = (u32)file_org_len;
		u32 unpack_offset = 0;

		if (fe[0].compress_flag)
			unCom(unpack, &unpack_len, cipher, file_pkg_len);
		else
			memcpy(unpack, cipher, file_org_len);
//*****************************************************************************//
		do{
			if (!p_decode) break;
			else if (p_decode == (_XOR_DECODE)0x80000000)
				xp3filter_decode(game, fe[0].file_name, unpack, file_org_len, unpack_offset, file_org_len, fe[0].crc);
			else
				p_decode(fe[0].crc, simple_xor_game[game_idx].extend_key, simple_xor_game[game_idx].offset, unpack, file_org_len);

		}while(0);
//*****************************************************************************//
		if (!SplitFileNameAndSave(cur_dir, fe[0].file_name, unpack, file_org_len))
			++cnt_savefile;

		VirtualFree(unpack, 0, MEM_RELEASE);
		VirtualFree(cipher, 0, MEM_RELEASE);
	}

	return cnt_savefile;
}

static void xor_decode(DWORD hash, u8 extend_key, u32 offset, PBYTE buf, DWORD len)	// 从offset开始解
{
	for (int i=offset; i<len; ++i)
		buf[i] ^= (BYTE)hash ^ extend_key;
	return;
}

static void xor_decode_prettycation(DWORD hash, u8 extend_key, u32 offset, PBYTE buf, DWORD len)
{
	for (int i=offset; i<len; ++i)
		buf[i] ^= (BYTE)(hash>>0xc);
	return;
}

static void xor_decode_swansong(DWORD hash, u8 extend_key, u32 offset, PBYTE buf, DWORD len)
{
	BYTE ror = (BYTE)hash & 7, key = (BYTE)(hash >> 8);
	for (int i=offset; i<len; ++i)
	{
		buf[i] = buf[i] ^ key;
		buf[i] = buf[i] >> ror | buf[i] << (8-ror);
	}
	return;
}