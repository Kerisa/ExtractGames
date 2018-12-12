// BlackSheep.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <cassert>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>
#include "tp_stub.h"
#include <tlhelp32.h>
#include "utility/utility.h"
#include "../kirikiri/xp3.h"

using namespace std;

struct MapObj
{
	BOOL Access;
	wchar_t Path[MAX_PATH];
};
MapObj *g_Map;


char dataBuffer[8192];
HANDLE hFileMapping;
uint32_t Finish = 0;
uint32_t OldV2Link = 0;
iTVPFunctionExporter *exporter;
uint32_t retAddr;
uint32_t temp;


DWORD WINAPI Worker(LPVOID)
{
    MessageBox(NULL, "seems OK", NULL, 0);

    wstring drv, dir, file, ext;
    Utility::SplitPath(g_Map->Path, drv, dir, file, ext);
    wstring outDir = drv + dir + L"[extract] " + file + ext;
    CreateDirectoryW(outDir.c_str(), 0);

	EncryptedXP3 *xp3 = new EncryptedXP3();
	bool b = xp3->Open(g_Map->Path);
	assert("xp3 open failed." && b);
	vector<file_entry> entries = xp3->ExtractEntries(xp3->GetPlainIndexBytes());
    int count = 0, streamErr = 0;
	for (size_t i = 0; i < entries.size(); ++i)
	{
        const file_entry& fe = entries[i];
        ttstr name(xp3->FormatFileNameForIStream(fe).c_str());

        IStream* stream = TVPCreateIStream(name, 0);
        if (stream)
        {
            STATSTG state;
            DWORD flag = 1;
            if (SUCCEEDED(stream->Stat(&state, flag)))
            {
                size_t fileSize = state.cbSize.QuadPart;
                LPVOID buf = VirtualAlloc(NULL, fileSize, MEM_COMMIT, PAGE_READWRITE);
                ULONG R;
                if (SUCCEEDED(stream->Read(buf, fileSize, &R)))
                {
                    assert(R == fileSize);
                    int ret = EncryptedXP3::SplitFileNameAndSave(outDir, fe.file_name, vector<char>((char*)buf, (char*)buf + fileSize));
                    if (ret == 0)
                        ++count;
                    VirtualFree(buf, 0, MEM_RELEASE);
                }
                else
                {
                    ++streamErr;
                }
            }
            else
            {
                ++streamErr;
            }
        }
        else
        {
            ++streamErr;
        }
	}

    stringstream ss;
    ss << "finish(" << count << "/" << entries.size() << ", " << streamErr << " for stream error)";
    MessageBox(NULL, ss.str().c_str(), 0, 0);

	while (1)
	{
		Sleep(1);
	}
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
}

extern "C" __declspec(dllexport) HRESULT _stdcall V2Unlink()
{
    // V2Unlink 是插件和吉里吉里本体分离时调用的函数。
    MessageBox(NULL, "UnLoading", NULL, 0);
    // Stub使用完毕(必须编写)
    TVPUninitImportStub();

    return S_OK;
}

uint32_t old;
uint32_t addr;
char * func;

extern "C" __declspec(naked) void ProcAddressHook()
{

    __asm {
		push ebp
        mov ebp, esp
        mov old, eax
        mov eax, dword ptr [ebp+0x18]
		mov eax, [eax]
		mov addr, eax
        mov func, eax
		mov eax, DWORD ptr [ebp+0x10]
		cmp eax, 0
		je SKIP
		lea eax, [eax+4]
		mov eax, [eax]
		mov func, eax
SKIP:
    }

    if (!Finish)
    {
        if (!memcmp(func, "V2Link", 7))
        {
            OldV2Link = addr;
			__asm {
				mov eax, dword ptr[ebp+0x18]
				mov ecx, V2Link
				mov [eax], ecx
			}
            //old = (uint32_t)V2Link;
        }
    }

    __asm {
        mov esp, ebp
		pop ebp
        mov eax, old
        pop ebp
        retn 0x10
    }
}

DWORD WINAPI ThreadRoutine(LPVOID)
{
    MessageBox(NULL, "start sub thread", NULL, 0);

    HMODULE hNtdll = LoadLibrary("ntdll.dll");
    uint32_t OldAddress = (uint32_t)GetProcAddress(hNtdll, "LdrGetProcedureAddress");
    SIZE_T R;
    char temp[64];
    ReadProcessMemory(GetCurrentProcess(), (LPVOID)OldAddress, temp, sizeof(temp), &R);
    assert(R == sizeof(temp));
    for (int i = 0; i < sizeof(temp) - 4; ++i)
    {
        // pop ebp
        // retn 0x10
        if (!memcmp(&temp[i], "\x5d\xc2\x10\x00", 4))
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
                HANDLE t = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
                assert(t);
                BOOL b = ResumeThread(t);
				assert(b != -1);
                CloseHandle(t);
            }
        } while (Thread32Next(hSnap, &te));
    }

    CloseHandle(hSnap);
    return 0;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD Reason, LPVOID lpReserved)
{
    if (Reason == DLL_PROCESS_ATTACH)
    {
		hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, 0, 4096, "BlackSheep{13E025E2-B7AA-4141-9B4E-402CCB3C4F33}");
		assert(hFileMapping != NULL);
		g_Map = (MapObj*)MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 4096);
		assert(g_Map);
		g_Map->Access = TRUE;
        CreateThread(NULL, 0, ThreadRoutine, 0, 0, NULL);
    }
    return 1;
}