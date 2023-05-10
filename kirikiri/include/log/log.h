
#pragma once

#include <string>
#include <sstream>

namespace Log
{
  constexpr const int debug = 0;
  constexpr const int info = 1;
  constexpr const int warn = 2;
  constexpr const int error = 3;

  // filename: filename of log file
  // max_file_num: max numbers of log files
  // max_file_size: max size of single log file
  void Init(const char* filename = nullptr, int max_file_num = 1, int max_file_size = 10 * 1024 * 1024);

  void WriteLogToFile(const std::string& L);
  std::string GetTimeOfDay();
  const char* GetLevelMark(int level);
  uint32_t GetPid();
  uint32_t GetTid();
}

#define WRITE_LOG(level, x) do { \
	  char msg1[1024] = { 0 }; \
	  sprintf_s(msg1, sizeof(msg1), "[%s] [%s] ", \
            Log::GetTimeOfDay().c_str(), Log::GetLevelMark(level)); \
    std::stringstream ss; \
    ss << x; \
    std::string L = msg1; \
    L += ss.str(); \
	  Log::WriteLogToFile(L); \
  } while (0)

#define LOG_DEBUG(x)  WRITE_LOG(Log::debug, x)
#define LOG_INFO(x)   WRITE_LOG(Log::info,  x)
#define LOG_WARN(x)   WRITE_LOG(Log::warn,  x)
#define LOG_ERROR(x)  WRITE_LOG(Log::error, x)

