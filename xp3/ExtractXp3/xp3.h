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

static const char *unencry_game [] = {"kamitsure", "startlight"};
static const struct _SIMPLE_XOR
{
	char *name;
	u8 key;
}simple_xor_game [] = {"kuranokunchi", 0xCD, "amakoi", 0x0};

extern void AppendMsg(PTSTR szBuffer);

int is_xp3_file(HANDLE hFile);
u8* uncompress_xp3_idx(HANDLE hFile, u32 *idx_len, UNCOM unCom);
int xp3_extract_file_save(HANDLE hFile, u8 *xp3_idx, int idx_len, u32 *file_num, char *game, UNCOM unCom, wchar_t *cur_dir);