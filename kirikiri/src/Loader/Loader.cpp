// Loader.cpp : 協吶陣崙岬哘喘殻會議秘笥泣。
//

#include "stdafx.h"
#include <cassert>
#include <windows.h>

int main(int arc, char** argv)
{

    STARTUPINFO si = { 0 };
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    char cmdline[256];


    strcpy_s(cmdline, sizeof(cmdline), "D:\\Games\\9-nine-ここのつここのかここのいろ\\nine_kokoiro.org.exe");
    BOOL b = CreateProcess(NULL, cmdline, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, "D:\\Games\\9-nine-ここのつここのかここのいろ\\", &si, &pi);


    assert(b);
    
    LPVOID remote = VirtualAllocEx(pi.hProcess, NULL, 4096, MEM_COMMIT, PAGE_READWRITE);
    assert(remote);
    SIZE_T W;
    
    WriteProcessMemory(pi.hProcess, remote, "D:\\Games\\9-nine-ここのつここのかここのいろ\\BlackSheep_d.dll", 64, &W);

    assert(W == 64);
    DWORD tid = 0;
    HANDLE rThread = CreateRemoteThread(pi.hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, remote, CREATE_SUSPENDED, &tid);
    printf("tid: %#x\n", tid);

    b = ResumeThread(rThread);
    assert(b != -1);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(rThread);

    return 0;
}

