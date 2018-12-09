// BlackSheep.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <cassert>
#include <windows.h>
#include "tp_stub.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <tlhelp32.h>
#include "utility/utility.h"

using namespace std;

wchar_t strBuffer[512] = L"archive://fgimage.xp3/emotion.txt";


char dataBuffer[8192];

uint32_t Finish = 0;
uint32_t OldV2Link = 0;
iTVPFunctionExporter *exporter;
uint32_t retAddr;
uint32_t temp;


DWORD WINAPI Worker(LPVOID)
{
    MessageBox(NULL, "seems OK", NULL, 0);

    ttstr name(strBuffer);

    const char target[] = "IStream * ::TVPCreateIStream(const ttstr &,tjs_uint32)";
    void* CallIStreamStub = TVPGetImportFuncPtr(target);

    IStream* stream = TVPCreateIStream(name, 0);
    STATSTG state;
    DWORD flag = 1;
    stream->Stat(&state, flag);
    size_t fileSize = state.cbSize.QuadPart;
    LPVOID buf = VirtualAlloc(NULL, fileSize, MEM_COMMIT, PAGE_READWRITE);
    ULONG R;
    stream->Read(buf, fileSize, &R);
    assert(R == fileSize);
    ofstream out("emotion.txt", ios::binary);
    assert(out.is_open());
    out.write((char*)buf, fileSize);
    out.close();
    VirtualFree(buf, 0, MEM_RELEASE);


    return 0;
}

extern "C" __declspec(naked) HRESULT _stdcall V2Link(/*iTVPFunctionExporter *exporter*/)
{
    // V2Link 是插件和吉里吉里本体链接时会被调用的函数

    __asm {
        mov temp, eax

        mov eax, [esp + 0x0]
        mov retAddr, eax
        mov eax, [esp + 0x4]
        mov exporter, eax
        push eax
        call TVPInitImportStub
        add esp, 4
        
        xor eax, eax
        push eax
        push eax
        push eax
        push Worker
        push eax
        push eax
        call CreateThread

        mov Finish, 0xaa55aa55

        
        mov eax, temp
        jmp OldV2Link
    }

    //MessageBox(NULL, "Loading V2Link", NULL, 0);
    //assert(OldV2Link);

    //MessageBox(NULL, "call old V2Link finish", NULL, 0);
    
    // 初始化Stub(必须编写)
    //TVPInitImportStub(exporter);

    //Finish = true;
    //return S_OK;
}

extern "C" __declspec(dllexport) HRESULT _stdcall V2Unlink()
{
    // V2Unlink 是插件和吉里吉里本体分离时调用的函数。
    MessageBox(NULL, "UnLoading", NULL, 0);
    // Stub使用完毕(必须编写)
    TVPUninitImportStub();

    return S_OK;
}

extern "C" __declspec(naked) void ProcAddressHook()
{
    uint32_t old;
    char * func;
    __asm {
        pushad
        mov old, eax
        mov eax, dword ptr [ebp+0xc]
        mov func, eax
    }

    if (!Finish)
    {
        if (!memcmp(func, "V2Link", 7))
        {
            OldV2Link = old;
            old = (uint32_t)V2Link;
        }
    }

    __asm {
        popad
        mov eax, old
        pop ebp
        retn 0x8
    }
}

DWORD WINAPI ThreadRoutine(LPVOID)
{
    MessageBox(NULL, "start sub thread", NULL, 0);

    HMODULE hKer32 = LoadLibrary("kernel32.dll");
    uint32_t OldAddress = (uint32_t)GetProcAddress(hKer32, "GetProcAddress");
    SIZE_T R;
    char temp[64];
    ReadProcessMemory(GetCurrentProcess(), (LPVOID)OldAddress, temp, sizeof(temp), &R);
    assert(R == sizeof(temp));
    for (int i = 0; i < sizeof(temp) - 4; ++i)
    {
        // pop ebp
        // retn 0x8
        if (!memcmp(&temp[i], "\x5d\xc2\x08\x00", 4))
        {
            OldAddress += i;
            break;
        }
    }
    char OldBytes[5] = { 0 };
    ReadProcessMemory(GetCurrentProcess(), (LPVOID)OldAddress, OldBytes, sizeof(OldBytes), &R);
    assert(R == sizeof(OldBytes));
    char NewBytes[5] = { '\xE9' };

    *(uint32_t*)&NewBytes[1] = (uint32_t)ProcAddressHook - OldAddress - 5;
    SIZE_T W;
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)OldAddress, NewBytes, sizeof(NewBytes), &W);
    assert(W == sizeof(NewBytes));



    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    THREADENTRY32 te;
    te.dwSize = sizeof(te);
    if (Thread32First(hSnap, &te))
    {
        do
        {
            if (te.th32OwnerProcessID == GetCurrentProcessId() && te.th32ThreadID != GetCurrentThreadId())
            {
                HANDLE t = OpenThread(THREAD_RESUME, FALSE, te.th32ThreadID);
                assert(t);
                ResumeThread(t);
                CloseHandle(t);
            }
        } while (Thread32Next(hSnap, &te));
    }

    CloseHandle(hSnap);

    //__asm {
    //    push [ebp+0xc]
    //    call ProcAddressHook
    //    add esp, 4
    //    jmp OldAddress
    //}

#if 0
    ::Sleep(5000);
    while (1)
    {
        __asm {
            nop
            nop
            nop
            nop
            nop
            nop
            nop
            nop
            nop
            nop
            nop
            nop
            nop
            nop
            nop
            nop
            nop
            nop
            nop
            nop
            nop
            nop
            nop
        }
    }
    IStream* stream = 0;
    uint32_t fn_TVPCreateIStream = 0;
    ttstr filename;

    __asm {
        lea eax, filename
        mov edx, 0
        call fn_TVPCreateIStream
        mov stream, eax
    }

    STATSTG state;
    ULONG R;
    stream->Stat(&state, 1);
    stream->Read(dataBuffer, state.cbSize.QuadPart, &R);
#endif
    return 0;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD Reason, LPVOID lpReserved)
{
    if (Reason == DLL_PROCESS_ATTACH)
    {
        CreateThread(NULL, 0, ThreadRoutine, 0, 0, NULL);
    }
    return 1;
}