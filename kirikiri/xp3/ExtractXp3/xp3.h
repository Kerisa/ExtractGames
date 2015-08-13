#pragma once

#include <Windows.h>
#include "types.h"
#include "error.h"
#include "cxdec\cxdec.h"
#include "xp3filter_decode.h"

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
	wchar_t file_name[128];
};

extern void AppendMsg(PTSTR szBuffer);

int is_xp3_file(HANDLE hFile);
u8* uncompress_xp3_idx(HANDLE hFile, u32 *idx_len, UNCOM unCom);
int xp3_extract_file_save(HANDLE hFile, u8 *xp3_idx, int idx_len, u32 *file_num, char *game, UNCOM unCom, wchar_t *cur_dir);

typedef void (*_XOR_DECODE)(DWORD hash, u8 extend_key, u32 offset, PBYTE buf, DWORD len);
 void xor_decode				(DWORD hash, u8 extend_key, u32 offset, PBYTE buf, DWORD len);
 void xor_decode_prettycation	(DWORD hash, u8 extend_key, u32 offset, PBYTE buf, DWORD len);
 void xor_decode_swansong	(DWORD hash, u8 extend_key, u32 offset, PBYTE buf, DWORD len);


static const char unencry_game [] = "Unencrypted";		// 数据没有加密的游戏
static const struct _SIMPLE_XOR		// 数据xor过的游戏
{
	char *name;			// 游戏名
	void (*p_decode)(DWORD hash, u8 extend_key, u32 offset, PBYTE buf, DWORD len);	
	u8    extend_key;	// 除去hash外的额外的key
	u32   offset;		// 从数据的offset字节开始解码
}
simple_xor_game [] = { "kuranokunchi",	xor_decode,					0xCD, 0x0,
					   "amakoi",		xor_decode,					 0x0, 0x0,
					   "prettycation",	xor_decode_prettycation,	 0x0, 0x5,
					   "swansong",		xor_decode_swansong,		 0x0, 0x0,};