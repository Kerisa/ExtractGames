// Pal.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <array>
#include <thread>
#include <set>
#include <sstream>
#include <psapi.h>
#include <TlHelp32.h>

#include "dbgpal.h"
#include "picture.h"

using namespace std;

uint32_t Splite(const char * _str, const char * _delim, std::vector<std::string>& out)
{
    std::set<char> delim;
    for (const char *p = _delim; *p != '\0'; ++p)
        delim.insert(*p);

    int iLength = strlen(_str);
    int iSaveCount = 0;
    int iLastPos = 0, iPos = 0;
    for (int i = 0; i < iLength && iLastPos < iLength; )
    {
        auto it = delim.find(_str[i]);
        if (it != delim.end())
        {
            if (i > iLastPos)
            {
                out.push_back(std::string(&_str[iLastPos], &_str[i]));
                ++iSaveCount;
            }

            // iLastPos移至下一个非delim字符上
            for (iLastPos = i + 1;
                iLastPos < iLength && delim.find(_str[iLastPos]) != delim.end();
                ++iLastPos)
                ;
            i = iLastPos + 1;
            continue;
        }
        ++i;
    }
    if (iLastPos < iLength)
    {
        out.push_back(std::string(&_str[iLastPos], &_str[iLength]));
        ++iSaveCount;
    }

    return iSaveCount;
}

std::string FullPath(const std::string & in)
{
    vector<char> _full;
    _full.resize(1024);
    _fullpath(_full.data(), in.c_str(), _full.size());
    return _full.data();
}

void SplitPath(const string& in, string* drv, string* path, string* name, string* ext)
{
    vector<char> _drv;   _drv.resize(in.size());
    vector<char> _path;  _path.resize(1024);
    vector<char> _name;  _name.resize(in.size());
    vector<char> _ext;   _ext.resize(in.size());

    string _full = FullPath(in);
    _splitpath_s(_full.c_str(), _drv.data(), _drv.size(), _path.data(), _path.size(), _name.data(), _name.size(), _ext.data(), _ext.size());

    if (drv) *drv = _drv.data();
    if (path) *path = _path.data();
    if (name) *name = _name.data();
    if (ext) *ext = _ext.data();
}


// 获取目标进程首地址
DWORD GetProcessImageBaseAddr(DWORD dwPID)
{
    HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
    MODULEENTRY32 me32;

    hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
    if (hModuleSnap == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

    me32.dwSize = sizeof(MODULEENTRY32);
    if (!Module32First(hModuleSnap, &me32))
    {
        CloseHandle(hModuleSnap);    // 清除句柄对象
        return NULL;
    }

    CloseHandle(hModuleSnap);
    return (DWORD)me32.modBaseAddr;
}

UINT GetAlign2(UINT w)
{
    for (UINT i = 0; i < 32; ++i)
    {
        if ((1u << i) >= w)
        {
            return 1 << i;
        }
    }
    assert(0);
    return 0;
}

bool Save32BppToFile(char* data, int byteLength, int width, int height, const std::string& saveNameWithoutExt)
{
#if 1
    // png
    Alisa::Image img;
    if (!img.FromRawPixels(Alisa::PixelType_RGBA, width, height, data, byteLength, true))
        return false;
    img.ModifyPixels([](int, int, Alisa::Pixel& p) {
        uint8_t r = p.R;
        p.R = p.B;
        p.B = r;
    });
    return img.SaveTo(saveNameWithoutExt + ".png", Alisa::E_ImageType_Png);
#else
    // original bmp
    BITMAPFILEHEADER bfh = { 0 };
    bfh.bfType = 'MB';
    bfh.bfSize = byteLength + sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);
    bfh.bfOffBits = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);

    BITMAPINFOHEADER bih = { 0 };
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biBitCount = 32;
    bih.biPlanes = 1;
    bih.biHeight = -height;
    bih.biWidth = width;
    bih.biSizeImage = byteLength;

    ofstream out;
    out.open(saveNameWithoutExt + ".bmp", ios::binary);
    if (!out.is_open())
        return false;

    out.write((char*)&bfh, sizeof(BITMAPFILEHEADER));
    out.write((char*)&bih, sizeof(BITMAPINFOHEADER));
    out.write(data, byteLength);
    out.close();
    return true;
#endif
}

bool StartProcess(const string& exePath, const string& exeDir, bool suspend, PROCESS_INFORMATION* ppi)
{
    STARTUPINFO si = { 0 };
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_MINIMIZE;
    PROCESS_INFORMATION pi;
    char cmdLine[1024];
    strcpy_s(cmdLine, sizeof(cmdLine), exePath.c_str());

    int flag = (suspend ? CREATE_SUSPENDED : 0) | DEBUG_ONLY_THIS_PROCESS;
    BOOL Success = CreateProcess(NULL, cmdLine,
        NULL, NULL, FALSE, flag, NULL, exeDir.c_str(),
        &si, &pi);
    if (!Success)
    {
        printf("start exe failed.\n");
        return false;
    }

    if (!suspend)
    {
        CloseHandle(pi.hThread);
        pi.hThread = NULL;
    }
    *ppi = pi;
    return true;
}

bool IsMemoryExceed(HANDLE hProcess, uint32_t bytes)
{
    PROCESS_MEMORY_COUNTERS pmc;
    if (hProcess)
    {
        BOOL b1 = GetProcessMemoryInfo(hProcess, &pmc, sizeof(PROCESS_MEMORY_COUNTERS));
        assert(b1);
        if (pmc.PeakWorkingSetSize > bytes)
        {
            return true;
        }
    }
    return false;
}


void ResetBreakPoint(DEBUG_EVENT* de, DebugInfo* di)
{
    if (di->hProcess == NULL)
        return;

    for (size_t i = 0; i < di->mBP.size(); ++i)
    {
        if (di->mBP[i].mNeedResetBP)
        {
            HANDLE hT = OpenThread(THREAD_SET_CONTEXT | THREAD_GET_CONTEXT, FALSE, de->dwThreadId);
            assert(hT);
            CONTEXT ctx;
            ctx.ContextFlags = CONTEXT_FULL;
            BOOL b = GetThreadContext(hT, &ctx);
            assert(b);
            CloseHandle(hT);

            SIZE_T D;
            ReadProcessMemory(di->hProcess, (LPVOID)di->mBP[i].mAddr, &di->mBP[i].mOldByte, 1, &D);
            WriteProcessMemory(di->hProcess, (LPVOID)di->mBP[i].mAddr, &di->int3, 1, &D);
            di->mBP[i].mNeedResetBP = false;
        }
    }
}


void TriggerBreakPoint(DEBUG_EVENT* de, DebugInfo* di)
{
    if (di->hProcess == NULL)
        return;

    for (size_t i = 0; i < di->mBP.size(); ++i)
    {
        if (de->u.Exception.ExceptionRecord.ExceptionAddress == (LPVOID)di->mBP[i].mAddr)
        {
            di->mBP[i].Handler(&di->mBP[i], de);
            break;
        }
    }
}


void StopDebug(DebugInfo* di)
{
    TerminateProcess(di->hProcess, 0);
}


bool ModifyThreadContext(DWORD tid, std::function<bool(CONTEXT& ctx)> f)
{
    // f return true to set context back

    HANDLE hT = OpenThread(THREAD_SET_CONTEXT | THREAD_GET_CONTEXT, FALSE, tid);
    if (hT == NULL)
        return false;

    CONTEXT ctx;
    ctx.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext(hT, &ctx))
    {
        CloseHandle(hT);
        return false;
    }

    bool result = f(ctx);

    if (result)
    {
        if (!SetThreadContext(hT, &ctx))
        {
            CloseHandle(hT);
            return false;
        }
    }

    CloseHandle(hT);
    return true;
}

void InitBreakPoints(DEBUG_EVENT* de, DebugInfo* di)
{
    SIZE_T D;
    di->mBP[0].mRemoteMemory = VirtualAllocEx(di->hProcess, NULL, 4096, MEM_COMMIT, PAGE_READWRITE);
    assert(di->mBP[0].mRemoteMemory);
    di->mBP[0].mAddr = di->mExeImageBase + 0x2f908;    // 0091F908  53  push ebx
    ReadProcessMemory(di->hProcess, (LPVOID)di->mBP[0].mAddr, &di->mBP[0].mOldByte, 1, &D);
    WriteProcessMemory(di->hProcess, (LPVOID)di->mBP[0].mAddr, &DebugInfo::int3, 1, &D);
    di->mBP[0].Handler = [di](BreakPointInfo* self, DEBUG_EVENT* de) {
        SIZE_T D;
        if (di->mCurrentImgIdx != -1)
        {
            assert("error image not saved" && 0);
        }
        for (size_t i = 0; i < di->mImgList.size(); ++i)
        {
            if (!di->mImgList[i].handled)
            {
                WriteProcessMemory(di->hProcess, self->mRemoteMemory, di->mImgList[i].name.c_str(), di->mImgList[i].name.size() + 1, &D);
                di->mCurrentImgIdx = i;
                break;
            }
        }
        if (di->mCurrentImgIdx == -1)
        {
            cout << "all images saved.\n";
            StopDebug(di);
        }

        WriteProcessMemory(di->hProcess, (LPVOID)self->mAddr, &self->mOldByte, 1, &D);
        bool b = ModifyThreadContext(de->dwThreadId, [self](CONTEXT& ctx) {
            --ctx.Eip;
            ctx.EFlags |= 0x100;        // 设单步以便重新设置断点
            ctx.Ebx = (DWORD)self->mRemoteMemory;
            return true;
        });
        assert(b);
        self->mNeedResetBP = true;
    };

    // 获取基础解码数据点1
    di->mBP[1].mAddr = (DWORD)di->mDllBaseAddr + 0xa46d;
    ReadProcessMemory(di->hProcess, (LPVOID)di->mBP[1].mAddr, &di->mBP[1].mOldByte, 1, &D);
    WriteProcessMemory(di->hProcess, (LPVOID)di->mBP[1].mAddr, &DebugInfo::int3, 1, &D);
    di->mBP[1].Handler = [di](BreakPointInfo* self, DEBUG_EVENT* de) {
        SIZE_T D;
        DWORD espVal = 0;
        bool b = ModifyThreadContext(de->dwThreadId, [self, &espVal](CONTEXT& ctx) {
            --ctx.Eip;
            ctx.EFlags |= 0x100;        // 设单步以便重新设置断点
            espVal = ctx.Esp;
            return true;
        });
        assert(b);

        LPVOID buffer = NULL;
        UINT width = 0;
        UINT height = 0;
        ReadProcessMemory(di->hProcess, (LPVOID)(espVal + 4), &buffer, 4, &D);
        ReadProcessMemory(di->hProcess, (LPVOID)(espVal + 8), &width, 4, &D);
        ReadProcessMemory(di->hProcess, (LPVOID)(espVal + 12), &height, 4, &D);

        //CloseHandle(hT);
        WriteProcessMemory(di->hProcess, (LPVOID)self->mAddr, &self->mOldByte, 1, &D);
        self->mNeedResetBP = true;

        assert(di->mCurrentImgIdx >= 0 && di->mCurrentImgIdx < di->mImgList.size());
        UINT w2 = GetAlign2(width);
        UINT length = w2 * height * 4;
        di->mImgList[di->mCurrentImgIdx].width = w2;
        di->mImgList[di->mCurrentImgIdx].height = height;
        di->mImgList[di->mCurrentImgIdx].dataLength = length;
        di->mImgList[di->mCurrentImgIdx].remoteBufferAddr = buffer;
        if (di->mImgList[di->mCurrentImgIdx].type == ImageInfo::Base)
        {
            PBYTE saveBuffer = (PBYTE)VirtualAlloc(NULL, length, MEM_COMMIT, PAGE_READWRITE);
            ReadProcessMemory(di->hProcess, buffer, saveBuffer, length, &D);

            Save32BppToFile((char*)saveBuffer, length, w2, height, di->mSaveDir + di->mImgList[di->mCurrentImgIdx].name);
            VirtualFree(saveBuffer, 0, MEM_RELEASE);

            cout << di->mImgList[di->mCurrentImgIdx].name << " saved.\n";

            di->mImgList[di->mCurrentImgIdx].handled = true;
            di->mCurrentImgIdx = -1;

            // 每次保存后判断内存使用量是否过多
            if (IsMemoryExceed(di->hProcess, DebugInfo::DebugeeMemoryLimit))
            {
                StopDebug(di);
            }
        }
    };


    // 获取基础解码数据点2，与 1 除地址外一样
    di->mBP[4].mAddr = (DWORD)di->mDllBaseAddr + 0xa4ab;
    ReadProcessMemory(di->hProcess, (LPVOID)di->mBP[4].mAddr, &di->mBP[4].mOldByte, 1, &D);
    WriteProcessMemory(di->hProcess, (LPVOID)di->mBP[4].mAddr, &DebugInfo::int3, 1, &D);
    di->mBP[4].Handler = di->mBP[1].Handler;


    // 获取解码数据点2, 适用带差分的图像
    di->mBP[3].mAddr = (DWORD)di->mDllBaseAddr + 0x26b82;
    ReadProcessMemory(di->hProcess, (LPVOID)di->mBP[3].mAddr, &di->mBP[3].mOldByte, 1, &D);
    WriteProcessMemory(di->hProcess, (LPVOID)di->mBP[3].mAddr, &DebugInfo::int3, 1, &D);
    di->mBP[3].Handler = [di](BreakPointInfo* self, DEBUG_EVENT* de) {
        SIZE_T D;
        bool b = ModifyThreadContext(de->dwThreadId, [self](CONTEXT& ctx) {
            --ctx.Eip;
            ctx.EFlags |= 0x100;        // 设单步以便重新设置断点
            return true;
        });
        assert(b);


        WriteProcessMemory(di->hProcess, (LPVOID)self->mAddr, &self->mOldByte, 1, &D);
        self->mNeedResetBP = true;
        

        assert(di->mCurrentImgIdx >= 0 && di->mCurrentImgIdx < di->mImgList.size());
        auto& img = di->mImgList[di->mCurrentImgIdx];
        assert(di->mImgList[di->mCurrentImgIdx].type == ImageInfo::Diff);
        {
            PBYTE saveBuffer = (PBYTE)VirtualAlloc(NULL, img.dataLength, MEM_COMMIT, PAGE_READWRITE);
            ReadProcessMemory(di->hProcess, img.remoteBufferAddr, saveBuffer, img.dataLength, &D);
            Save32BppToFile((char*)saveBuffer, img.dataLength, img.width, img.height, di->mSaveDir + img.name);
            VirtualFree(saveBuffer, 0, MEM_RELEASE);

            cout << di->mImgList[di->mCurrentImgIdx].name << " saved.\n";
            img.handled = true;
            di->mCurrentImgIdx = -1;

            // 每次保存后判断内存使用量是否过多
            if (IsMemoryExceed(di->hProcess, DebugInfo::DebugeeMemoryLimit))
            {
                StopDebug(di);
            }
        }
    };

    // 解码函数完成后返回时中断，重设 eip 继续解码
    di->mBP[2].mAddr = di->mExeImageBase + 0x2f930;    // 0091F930  test eax, eax
    ReadProcessMemory(di->hProcess, (LPVOID)di->mBP[2].mAddr, &di->mBP[2].mOldByte, 1, &D);
    WriteProcessMemory(di->hProcess, (LPVOID)di->mBP[2].mAddr, &DebugInfo::int3, 1, &D);
    di->mBP[2].Handler = [di](BreakPointInfo* self, DEBUG_EVENT* de) {
        SIZE_T D;
        WriteProcessMemory(di->hProcess, (LPVOID)self->mAddr, &self->mOldByte, 1, &D);

        bool b = ModifyThreadContext(de->dwThreadId, [self](CONTEXT& ctx) {
            ctx.Eip -= (0x3a + 1);        // 回退以便继续下一个
            ctx.EFlags |= 0x100;        // 设单步以便重新设置断点
            ctx.Eax = (DWORD)self->mRemoteMemory;
            return true;
        });
        assert(b);
        self->mNeedResetBP = true;
    };
}

bool DebugIt(const string& exePath, const string& exeDir, const string& saveDir, vector<ImageInfo>& imgList)
{
    PROCESS_INFORMATION pi;
    if (!StartProcess(exePath, exeDir, false, &pi))
        return false;

    DebugInfo di(imgList);
    di.hProcess = pi.hProcess;
    di.mSaveDir = saveDir;
    
    DEBUG_EVENT de;
    while (1)
    {
        if (di.mStopDebugger)
        {
            DWORD r = WaitForSingleObject(di.hProcess, INFINITE);
            assert(r == WAIT_OBJECT_0);
            CloseHandle(di.hProcess);
            di.hProcess = NULL;
            return true;
        }

        BOOL b1 = WaitForDebugEvent(&de, 5000);
        assert(b1);
        
        if (de.dwDebugEventCode == LOAD_DLL_DEBUG_EVENT)
        {
            // 加载地址 0x10000000
            if (de.u.LoadDll.lpBaseOfDll == (LPVOID)DebugInfo::PalDllBase)
            {
                HANDLE hPalDll = de.u.LoadDll.hFile;
                CloseHandle(hPalDll);
                di.mDllBaseAddr = de.u.LoadDll.lpBaseOfDll;

                // 调用 dll 的入口处，位于 exe 中
                di.mExeImageBase = GetProcessImageBaseAddr(de.dwProcessId);
                assert(di.mExeImageBase);

                InitBreakPoints(&de, &di);
            }
        }
        else if (de.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
        {
            if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_SINGLE_STEP)
            {
                ResetBreakPoint(&de, &di);
            }
            else if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_BREAKPOINT)
            {
                TriggerBreakPoint(&de, &di);
            }
        }
        else if (de.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT)
        {
            di.mStopDebugger = true;
        }


        b1 = ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE);
        assert(b1);
    }

    return false;
}