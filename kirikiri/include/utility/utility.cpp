#pragma once

#include "utility.h"
#include <cassert>
#include <Windows.h>

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
