#pragma once

#include <cstdint>
#include <Windows.h>
#include <string>
#include <vector>
#include "detours/detours.h"

namespace Utility
{
    int          SearchMemory(HANDLE hProcess, uint32_t from, uint32_t to, uint32_t value, uint32_t* result, uint32_t arrayCount);
    int          SearchMemory(HANDLE hProcess, uint32_t from, uint32_t to, const char* str, uint32_t strLength, uint32_t* result, uint32_t arrayCount);
    uint8_t*     SearchSequence(uint8_t* search_start, uint32_t search_length, const std::vector<char>& pattern);

    bool         SplitPath(const std::string& full, std::string& drive, std::string& dir, std::string& file, std::string& ext);
    bool         SplitPath(const std::wstring& full, std::wstring& drive, std::wstring& dir, std::wstring& file, std::wstring& ext);
    std::string  MakeFullPath(const std::string& relative);
    std::wstring MakeFullPath(const std::wstring& relative);
    std::string  GetPathDir(const std::string& path);
    std::wstring GetPathDir(const std::wstring& path);
    std::string  GetExeDirA();
    std::wstring GetExeDirW();

    std::wstring GBKToUnicode(const std::string& str);
    std::wstring UTF8ToUnicode(const std::string& str);
    std::string  UnicodeToGBK(const std::wstring& str);
    std::string  UnicodeToUTF8(const std::wstring& str);

    std::string  GetTimeString();
    std::string  GetTimeFmtString(const char* fmt);

    void         ResumeOtherThread();

    std::string  GenTmpFilePath();

    template <class T>
    void AddHook(T* original_func, T detoured_func) {
      if (DetourTransactionBegin() == NO_ERROR) {
        DetourAttach((PVOID*)original_func, detoured_func);
        DetourTransactionCommit();
      }
    }

    template <class T>
    void RemoveHook(T* original_func, T detoured_func) {
      if (DetourTransactionBegin() == NO_ERROR) {
        DetourDetach((PVOID*)original_func, detoured_func);
        DetourTransactionCommit();
      }
    }
}
