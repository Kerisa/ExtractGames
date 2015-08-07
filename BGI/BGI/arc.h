#pragma once

#include <windows.h>
#include "type.h"
#include "error.h"

#define ARC_FILE_TYPE 2

struct IDX
{
	u8 name[96];
	u32 offset;
	u32 size;
};

static const char *arc_file_type[] = {"PackFile    ", "BURIKO ARC20"};
static const int magic_len[] = {12, 12};

struct IDX* get_arc_idx_0(HANDLE hFile, u32 *file_num, u32 *idx_size);
struct IDX* get_arc_idx_1(HANDLE hFile, u32 *file_num, u32 *idx_size);

typedef struct IDX* (*GET_IDX)(HANDLE hFile, u32 *file_num, u32 *idx_size);


int Is_arc(HANDLE hFile);
int arc_extract_file_save(HANDLE hFile,const struct IDX * const arc_idx, int file_num, u32 correct, wchar_t *cur_dir);
int SplitFileNameAndSave(wchar_t *cur_dir, wchar_t *file_name, void* unpack, u32 file_length);


extern void AppendMsg(wchar_t* szBuffer);


extern int Is_DSC(const u8* in);
extern int DecodeDSC(u8** raw_data, u8* in, int in_size);
extern int Is_DSC_Image(const u8 const *raw_data, u32 len);
extern u8* Save_DSC_Bmp(const u8 const *raw_data, u32 len, u32 *bmp_len);

extern int Is_CBG(const u8* in);
extern int DecodeCBG(u8** raw_data, u8* in, int in_size);