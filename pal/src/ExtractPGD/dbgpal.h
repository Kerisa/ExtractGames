#pragma once

#include <array>
#include <functional>
#include <vector>
#include <string>
#include <windows.h>

struct ImageInfo
{
    enum {
        Base = 0,
        Diff = 1,
    };
    ImageInfo(const std::string& n, int t) : name(n), type(t) { }
    std::string name;
    DWORD size{ 0 };
    DWORD offset{ 0 };

    int     width{ 0 };
    int     height{ 0 };
    int     dataLength;
    LPVOID  remoteBufferAddr{ 0 };
    int     type{ Base };
    bool    handled{ false };
};


struct BreakPointInfo
{
    BYTE    mOldByte{ 0 };
    DWORD   mAddr{ 0 };
    bool    mNeedResetBP{ false };
    LPVOID  mRemoteMemory{ NULL };
    std::function<void(BreakPointInfo*, DEBUG_EVENT*)> Handler;
};

struct DebugInfo
{
    static const int MaxBPCount = 5;
    static const BYTE int3 = 0xcc;
    static const int DebugeeMemoryLimit = 1024 * 1024 * 600;
    static const DWORD PalDllBase = 0x10000000;

    HANDLE  hProcess{ NULL };
    LPVOID  mDllBaseAddr{ NULL };
    DWORD   mExeImageBase{ NULL };
    int     mCurrentImgIdx{ -1 };
    bool    mStopDebugger{ false };
    std::vector<ImageInfo>&                 mImgList;
    std::array<BreakPointInfo, MaxBPCount>  mBP;
    std::string                             mSaveDir;

    DebugInfo(std::vector<ImageInfo>& imgs) : mImgList(imgs) { }
};



uint32_t Splite(const char * _str, const char * _delim, std::vector<std::string>& out);
std::string FullPath(const std::string& in);
void SplitPath(const std::string& in, std::string* drv, std::string* path, std::string* name, std::string* ext);
bool DebugIt(const std::string& exePath, const std::string& exeDir, const std::string& saveDir, std::vector<ImageInfo>& imgList);