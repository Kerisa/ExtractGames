// Loader.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <windows.h>
#include "utility/utility.h"
#include "../BlackSheep/ShareMem.h"

using namespace std;


int main(int argc, char** argv)
{
    if (argc < 3)
    {
        string file;
        Utility::SplitPath(argv[0], string(), string(), file);
        cout << "usage: " << file << " <game exe> [--passive] <xp3 file1> <xp3 file2>...\n"
          << "   passive    dump all resources while playing game instead of extract them directly\n";
        return 1;
    }
    else if (argc > _countof(MapObj::Path) + 2)
    {
        cout << "too many xp3 file.\n";
        return 1;
    }

    wstring exePath(Utility::MakeFullPath(Utility::GBKToUnicode(argv[1])));
    wstring exeDir(Utility::GetPathDir(exePath));
#ifdef _DEBUG
    wstring dllPath(Utility::GetExeDirW() + L"BlackSheep_d.dll");
#else
    wstring dllPath(Utility::GetExeDirW() + L"BlackSheep.dll");
#endif
    const bool passive_mode = (string(argv[2]) == "--passive");
    vector<wstring> allXp3Path;
    for (int i = passive_mode ? 3 : 2; i < argc; ++i)
    {
        allXp3Path.push_back(Utility::MakeFullPath(Utility::GBKToUnicode(argv[i])));
        if (allXp3Path.back().size() >= _countof(MapObj::Path[0]))
        {
            cout << "xp3 path too long.\n";
            return 1;
        }
    }
    assert(allXp3Path.size() <= _countof(MapObj::Path));


    STARTUPINFOW si = { 0 };
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    wchar_t cmdline[256];

    wcscpy_s(cmdline, _countof(cmdline), exePath.c_str());
    BOOL b = CreateProcessW(NULL, cmdline, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, exeDir.c_str(), &si, &pi);
    assert(b);

    
    LPVOID remote = VirtualAllocEx(pi.hProcess, NULL, 4096, MEM_COMMIT, PAGE_READWRITE);
    assert(remote);
    

    SIZE_T W;
    WriteProcessMemory(pi.hProcess, remote, dllPath.c_str(), (dllPath.size() + 1) * sizeof(wchar_t), &W);
    assert(W == (dllPath.size() + 1) * sizeof(wchar_t));


    DWORD tid = 0;
    HANDLE rThread = CreateRemoteThread(pi.hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryW, remote, CREATE_SUSPENDED, &tid);
    printf("pid: %#x, tid: %#x, dll_tid:%#x\n", pi.dwProcessId, pi.dwThreadId, tid);


    SIZE_T MapSize = sizeof(MapObj);
    HANDLE hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, 0, MapSize, "BlackSheep{13E025E2-B7AA-4141-9B4E-402CCB3C4F33}");
    LPVOID Addr = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, MapSize);
    memset(Addr, 0, MapSize);
    MapObj* mo = (MapObj*)Addr;
    mo->Access = false;
    mo->Count = allXp3Path.size();
    mo->PassiveMode = passive_mode;
    for (size_t i = 0; i < allXp3Path.size(); ++i)
    {
        wcscpy_s(mo->Path[i], _countof(mo->Path[i]), allXp3Path[i].c_str());
    }
    

    b = ResumeThread(rThread);
    assert(b != -1);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(rThread);

    while (((MapObj*)Addr)->Access != true)
    {
        Sleep(1);
    }
    UnmapViewOfFile(Addr);
    CloseHandle(hMap);
    return 0;
}

