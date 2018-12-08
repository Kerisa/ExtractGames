#pragma once


#include <array>
#include <functional>
#include <vector>
#include <string>
#include <windows.h>
#include "../kirikiri/xp3.h"

struct FileInfo
{
    std::string  mFilename;
    uint64_t     mOffset;
    uint32_t     mPackSize;
	uint32_t     mOrigSize;
    uint32_t     mChecksum;
    bool         mCompressed;
    bool         mEncrypted;
};

struct BreakPointInfo
{
    BYTE    mOldByte{ 0 };
    bool    mNeedResetBP{ false };
	bool	mBpCondition{ false };
	DWORD   mAddr{ 0 };
    LPVOID  mRemoteMemory{ NULL };
	void*	mUserData{ nullptr };
    std::function<void(BreakPointInfo*, DEBUG_EVENT*)> mHandler;
};


struct DebugInfo
{
    static constexpr int MaxBPCount = 5;
    static constexpr BYTE int3 = 0xcc;

	std::string mExePath;
	std::string mExeDir;
	std::string mSaveDir;
    std::string mXp3Path;

    HANDLE      hProcess{ NULL };
    DWORD       mExeImageBase{ NULL };
    bool        mStopDebugger{ false };
	DWORD		mCxdecVtbl{ 0 };
	DWORD		mCxdecCon{ 0 };
    file_entry  mStartupFile;
	std::pair<int, int> mKeyPair;
    std::array<BreakPointInfo, MaxBPCount>  mBP;
    std::vector<file_entry>                   mFileList;

    DebugInfo(HANDLE process, std::vector<file_entry>& list, file_entry startuptjs) : hProcess(process), mFileList(list), mStartupFile(startuptjs) { }
};




class Xp3Fucker
{
public:
};


bool DebugIt(const std::string& exePath, const std::string& exeDir, const std::string& xp3Path, const std::string& saveDir, std::vector<file_entry>& fileList, file_entry& startuptjs);
