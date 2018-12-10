#pragma once

#include <cstdint>
#include <Windows.h>
#include <string>

int SearchMemory(HANDLE hProcess, uint32_t from, uint32_t to, uint32_t value, uint32_t* result, uint32_t arrayCount);
int SearchMemory(HANDLE hProcess, uint32_t from, uint32_t to, const char* str, uint32_t strLength, uint32_t* result, uint32_t arrayCount);

bool SplitPath(const std::string& full, std::string& drive = std::string(), std::string& dir = std::string(), std::string& file = std::string(), std::string& ext = std::string());
std::string MakeFullPath(const std::string& relative);
std::string GetPathDir(const std::string& path);
std::string GetExeDir();

