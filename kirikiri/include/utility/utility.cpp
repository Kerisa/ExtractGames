#pragma once

#include "utility.h"
#include <cassert>
#include <chrono>
#include <vector>
#include <Windows.h>
#include <tlhelp32.h>

using namespace std;

namespace Utility
{


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

uint8_t* SearchSequence(uint8_t* search_start, uint32_t search_length, const vector<char>& pattern) {

  for (uint8_t* search_end = search_start + search_length; search_start < search_end - pattern.size(); ++search_start)
  {
    bool found = std::equal(search_start, search_start + pattern.size(), pattern.begin(), pattern.end(), [](char L, char R) {
      return L == R || R == '?';
      });
    if (found)
      return search_start;
  }

  return nullptr;
}

bool SplitPath(const std::string & full, std::string & drive, std::string & dir, std::string & file, std::string & ext)
{
	char _drive[MAX_PATH] = { 0 }, _dir[MAX_PATH] = { 0 }, _file[MAX_PATH] = { 0 }, _ext[MAX_PATH] = { 0 };
	errno_t err = _splitpath_s(full.c_str(), _drive, sizeof(_drive), _dir, sizeof(_dir), _file, sizeof(_file), _ext, sizeof(_ext));
	if (err != 0)
		return false;

	drive = _drive;
	dir = _dir;
	file = _file;
	ext = _ext;
	return true;
}

bool SplitPath(const std::wstring & full, std::wstring & drive, std::wstring & dir, std::wstring & file, std::wstring & ext)
{
    wchar_t _drive[MAX_PATH] = { 0 }, _dir[MAX_PATH] = { 0 }, _file[MAX_PATH] = { 0 }, _ext[MAX_PATH] = { 0 };
    errno_t err = _wsplitpath_s(full.c_str(), _drive, _countof(_drive), _dir, _countof(_dir), _file, _countof(_file), _ext, _countof(_ext));
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
    char buf[MAX_PATH] = { 0 };
	char* file = NULL;
	GetFullPathNameA(relative.c_str(), sizeof(buf), buf, &file);
	return buf;
}

std::wstring MakeFullPath(const std::wstring & relative)
{
    wchar_t buf[MAX_PATH] = { 0 };
    wchar_t* file = NULL;
    GetFullPathNameW(relative.c_str(), _countof(buf), buf, &file);
    return buf;
}

std::string GetPathDir(const std::string& path)
{
	string drv, dir;
	if (!SplitPath(MakeFullPath(path), drv, dir))
		return std::string();

	return drv + dir;
}

std::wstring GetPathDir(const std::wstring& path)
{
    wstring drv, dir;
    if (!SplitPath(MakeFullPath(path), drv, dir))
        return std::wstring();

    return drv + dir;
}

std::string GetExeDirA()
{
	char buf[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, buf, sizeof(buf));

	return GetPathDir(buf);
}

std::wstring GetExeDirW()
{
    wchar_t buf[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, buf, _countof(buf));

    return GetPathDir(buf);
}



wstring GBKToUnicode(const string& str)
{
	int size = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, 0, 0);
	vector<wchar_t> wcs(size + 1);
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wcs.data(), wcs.size());
	return wcs.data();
}

std::string UnicodeToGBK(const std::wstring& str)
{
	int size = WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, 0, 0, 0, 0);
	vector<char> mbs(size + 1);
	WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, mbs.data(), mbs.size(), NULL, NULL);
	return mbs.data();
}

std::wstring UTF8ToUnicode(const std::string & str)
{
	int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, 0, 0);
	vector<wchar_t> wcs(size + 1);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wcs.data(), wcs.size());
	return wcs.data();
}

std::string UnicodeToUTF8(const std::wstring & str)
{
	int size = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, 0, 0, 0, 0);
	vector<char> mbs(size + 1);
	WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, mbs.data(), mbs.size(), NULL, NULL);
	return mbs.data();
}

std::string GetTimeString() {
  std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::string s(32, '\0');
  tm tmmmmm;
  localtime_s(&tmmmmm, &now);
  std::strftime(&s[0], s.size(), "%Y-%m-%d %H:%M:%S", &tmmmmm);
  return s.c_str();
}

std::string GetTimeFmtString(const char* fmt)
{
  time_t tv;
  struct tm tm;
  char buf[32] = { 0 };

  time(&tv);
  localtime_s(&tm, &tv);
  strftime(buf, _countof(buf), fmt, &tm);
  return buf;
}

void ResumeOtherThread()
{
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  THREADENTRY32 te;
  te.dwSize = sizeof(te);
  if (Thread32First(hSnap, &te))
  {
    do
    {
      if (te.th32OwnerProcessID == GetCurrentProcessId() && te.th32ThreadID != GetCurrentThreadId())
      {
        HANDLE t = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
        assert(t);
        BOOL b = ResumeThread(t);
        assert(b != -1);
        CloseHandle(t);
      }
    } while (Thread32Next(hSnap, &te));
  }

  CloseHandle(hSnap);
  return;
}

}