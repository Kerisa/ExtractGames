#ifndef XP3FILTER_DECODE_H
#define XP3FILTER_DECODE_H

#include <windows.h>
#include <tchar.h>
#include <shlwapi.h>
#include <stdio.h>

struct xp3filter {
    const char *parameter;
    const WCHAR *resource_name;
    BYTE *buffer;
    DWORD length;
    DWORD offset;
    DWORD total_length;
    DWORD hash;
};

extern void xp3filter_decode_init(void);
extern void xp3filter_decode(char *game, const WCHAR *name, BYTE *buf, DWORD len, DWORD offset, DWORD total_len, DWORD hash);
extern void xp3filter_post_decode(const WCHAR *name, BYTE *buf, DWORD len, DWORD hash);

#endif    /* XP3FILTER_DECODE_H */
