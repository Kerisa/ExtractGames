
#pragma once

#include <Windows.h>

extern void AppendMsg(HWND hOut, const wchar_t *szBuffer);
extern int SplitFileNameAndSave(const wchar_t *cur_dir, const wchar_t *file_name, void* unpack, unsigned long  file_length);
