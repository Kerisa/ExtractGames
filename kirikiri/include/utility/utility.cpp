#pragma once

#include "utility.h"
#include <cassert>
#include <Windows.h>

using namespace std;

// 只按 4 字节对齐搜索
int SearchMemory(HANDLE hProcess, uint32_t from, uint32_t to, uint32_t value, uint32_t* result, uint32_t arrayCount)
{
    assert((from & 3) == 0);
    assert((to & 3) == 0);
    assert(arrayCount > 0);
    BYTE remoteMemory[4096];

    uint32_t index = 0;
    while (from < to && index < arrayCount)
    {
        SIZE_T D;
        DWORD length = ((from + 0x1000) & ~0xfff) - from;
        ReadProcessMemory(hProcess, (LPVOID)from, remoteMemory, length, &D);
        if (D == 0)
        {
            from = (from + 0x1000) & ~0xfff;
            continue;
        }

        for (DWORD p = 0; p < length; p += 4)
        {
            if (value == *(PDWORD)(remoteMemory + p))
            {
                result[index++] = from + p;
                assert(index < arrayCount);
            }
        }

        from += length;
    }
    return index;
}

int SearchMemory(HANDLE hProcess, uint32_t from, uint32_t to, const char* str, uint32_t strLength, uint32_t* result, uint32_t arrayCount)
{
    assert(strLength > 0);
    assert(arrayCount > 0);
    BYTE remoteMemory[4096];

    uint32_t index = 0;
    while (from < to && index < arrayCount)
    {
        SIZE_T D;
        DWORD length = ((from + 0x1000) & ~0xfff) - from;
        ReadProcessMemory(hProcess, (LPVOID)from, remoteMemory, length, &D);
        if (D == 0)
        {
            from = (from + 0x1000) & ~0xfff;
            continue;
        }

        for (DWORD p = 0; p < length; ++p)
        {
            if (!memcmp(&remoteMemory[p], str, strLength))
            {
                result[index++] = from + p;
                assert(index < arrayCount);
                p += strLength - 1;		// ++p
            }
        }

        from += length;
    }
    return index;
}

bool SplitPath(const std::string & full, std::string & drive, std::string & dir, std::string & file, std::string & ext)
{
	char _drive[MAX_PATH], _dir[MAX_PATH], _file[MAX_PATH], _ext[MAX_PATH];
	errno_t err = _splitpath_s(full.c_str(), _drive, sizeof(_drive), _dir, sizeof(_dir), _file, sizeof(_file), _ext, sizeof(_ext));
	if (err != 0)
		return false;

	drive = _drive;
	dir = _dir;
	file = _file;
	ext = _ext;
	return true;
}

std::string MakeFullPath(const std::string & relative)
{
	char buf[MAX_PATH];
	char* file = NULL;
	GetFullPathNameA(relative.c_str(), sizeof(buf), buf, &file);
	return buf;
}

std::string GetPathDir(const std::string& path)
{
	string drv, dir;
	if (!SplitPath(MakeFullPath(path), drv, dir))
		return std::string();

	return drv + dir;
}

std::string GetExeDir()
{
	char buf[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, buf, sizeof(buf));

	return GetPathDir(buf);
}
