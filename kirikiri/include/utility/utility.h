#pragma once

#include <cstdint>
#include <Windows.h>
#include <string>

namespace Utility
{
    int SearchMemory(HANDLE hProcess, uint32_t from, uint32_t to, uint32_t value, uint32_t* result, uint32_t arrayCount);
    int SearchMemory(HANDLE hProcess, uint32_t from, uint32_t to, const char* str, uint32_t strLength, uint32_t* result, uint32_t arrayCount);

    bool         SplitPath(const std::string& full, std::string& drive = std::string(), std::string& dir = std::string(), std::string& file = std::string(), std::string& ext = std::string());
    bool         SplitPath(const std::wstring& full, std::wstring& drive = std::wstring(), std::wstring& dir = std::wstring(), std::wstring& file = std::wstring(), std::wstring& ext = std::wstring());
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
}
