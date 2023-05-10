
#include <iostream>
#include <fstream>
#include <vector>

#include <chrono>
#include <iomanip>
#include <map>
#include <sstream>

#include "Log.h"
#include <windows.h>
#include "utility/utility.h"

using namespace std;

class LogSteam
{
public:
  static LogSteam* Instance()
  {
    static LogSteam inst;
    return &inst;
  }

  void Init(const char* file_name, int max_file_num, int max_file_size)
  {
    if (file_name)
      file_name_ = file_name;

    max_file_num_ = max_file_num;
    max_file_size_ = max_file_size;

  }

  int GetFileSize(const string& file_path)
  {
    ifstream file(file_path.data());
    if (!file.is_open())
    {
      return 0;
    }
    file.seekg(0, ios::end);
    int size = (int)file.tellg();
    file.close();

    return size;
  }

  string MakeFilePath(int index)
  {
    stringstream ss;
    ss << file_dir_;
    ss << '\\';
    ss << file_name_;
    ss << '_';
    ss << Utility::GetTimeFmtString("%Y%m%d%H%M%S");
    ss << '_';
    ss << GetCurrentProcessId();
    ss << '_';
    ss << index;
    ss << ".log";
    return ss.str();
  }

  string NextFilePath()
  {
    string path = MakeFilePath(file_index_);
    file_index_++;
    if (file_index_ == max_file_num_)
    {
      file_index_ = 0;
    }
    return path;
  }

  void WriteLog(const string& log_str)
  {
    string file_path;
    std::ios_base::openmode open_mode = ios::app;
    if (file_paths_.empty())
    {
      file_path = NextFilePath();
      file_paths_.push_back(file_path);
    }
    else
    {
      file_path = file_paths_.back();

      int size = GetFileSize(file_path);
      if (size > max_file_size_)
      {
        if (file_paths_.size() >= (size_t)max_file_num_)
        {
          auto first = file_paths_.begin();
          if (DeleteFileA(first->c_str()))
          {
            file_paths_.erase(first);
          }
        }

        file_path = NextFilePath();
        open_mode = ios::trunc;
        file_paths_.push_back(file_path);
      }
    }

    ofstream file(file_path.data(), open_mode);
    if (!file.is_open())
    {
      return;
    }

    file << log_str << endl;

    file.close();
  }

private:
  LogSteam()
  {
    char pszModuleName[512];
    memset(pszModuleName, 0, 512);
    HMODULE hModule = NULL;
    GetModuleHandleExA(
      GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
      (LPCSTR)&LogSteam::Instance, &hModule);
    if (hModule)
      GetModuleFileNameA(hModule, pszModuleName, sizeof(pszModuleName));

    std::string strFolder = pszModuleName;
    int nPos = strFolder.find_last_of('\\');
    file_name_ = strFolder.substr(nPos + 1);
    strFolder.erase(strFolder.begin() + nPos, strFolder.end());
    file_dir_ = strFolder;

    CreateDirectoryA(file_dir_.data(), NULL);

    for (int index = 0; index < max_file_num_; index++)
    {
      string file_path = MakeFilePath(index);
      int size = GetFileSize(file_path);
      if (size < max_file_size_)
      {
        file_index_ = index;
        break;
      }
    }
  }

  int max_file_num_{ 16 };
  int max_file_size_{ 10 * 1024 * 1024 };
  int file_index_{ 0 };
  string file_name_;
  string file_dir_;
  vector<string> file_paths_;
};


void Log::Init(const char* filename/*=nullptr*/, int max_file_num/*=16*/, int max_file_size/*=10240*/)
{
  LogSteam::Instance()->Init(filename, max_file_num, max_file_size);
}

void Log::WriteLogToFile(const std::string& L)
{
  LogSteam::Instance()->WriteLog(L);
}

std::string Log::GetTimeOfDay()
{
  std::ostringstream oss;
  auto now = std::chrono::system_clock::now();
  auto t = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
  tm fmt;
  localtime_s(&fmt, &t);
  oss << std::put_time(&fmt, "%Y/%m/%d %H:%M:%S");
  oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
  return oss.str();
}

const char* Log::GetLevelMark(int level)
{
  switch (level)
  {
  case 0:   return "D"; // debug
  case 2:   return "W"; // warning
  case 3:   return "E"; // error
  case 1:
  default:  return "I";     // info
  }
}

uint32_t Log::GetPid()
{
  return GetCurrentProcessId();
}

uint32_t Log::GetTid()
{
  return GetCurrentThreadId();
}