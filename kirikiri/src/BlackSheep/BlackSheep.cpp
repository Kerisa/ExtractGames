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
#include <VersionHelpers.h>
#include "utility/utility.h"
#include "../kirikiri/xp3.h"

using namespace std;

const char szAppName[] = "BlackSheep";

struct MapObj
{
	BOOL Access;
	wchar_t Path[MAX_PATH];
};
MapObj *g_Map;



HANDLE hFileMapping;
uint32_t Finish = 0;
uint32_t OldV2Link = 0;
iTVPFunctionExporter *exporter;
uint32_t retAddr;
uint32_t temp;
bool g_VerWin10;

DWORD WINAPI Worker(LPVOID)
{
    MessageBox(NULL, "seems OK, begin extracting, wait a minute....", szAppName, MB_ICONINFORMATION);

    wstring drv, dir, file, ext;
    Utility::SplitPath(g_Map->Path, drv, dir, file, ext);
    wstring outDir = drv + dir + L"[extract] " + file + ext;
    CreateDirectoryW(outDir.c_str(), 0);

	EncryptedXP3 *xp3 = new EncryptedXP3();
	bool b = xp3->Open(g_Map->Path);
	if (!b)
	{
		MessageBox(NULL, "xp3 open failed, exit", szAppName, MB_ICONWARNING);
		return 0;
	}
	vector<file_entry> entries = xp3->ExtractEntries(xp3->GetPlainIndexBytes());
    int count = 0, streamErr = 0;
	vector<string> report;
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
                if (SUCCEEDED(stream->Read(buf, fileSize, &R)) && R == fileSize)
                {
                    assert(R == fileSize);
                    int ret = EncryptedXP3::SplitFileNameAndSave(outDir, fe.file_name, vector<char>((char*)buf, (char*)buf + fileSize));
					if (ret == 0)
					{
						report.push_back(string("[success] saving [") + Utility::UnicodeToUTF8(fe.file_name) + "].");
						++count;
					}
					else
					{
						report.push_back(string("[failed] saving [") + Utility::UnicodeToUTF8(fe.file_name) + "].");
					}
                    VirtualFree(buf, 0, MEM_RELEASE);
                }
                else
                {
					report.push_back(string("[failed] reading [") + Utility::UnicodeToUTF8(fe.file_name) + "].");
                    ++streamErr;
                }
            }
            else
            {
				report.push_back(string("[failed] error stream [") + Utility::UnicodeToUTF8(fe.file_name) + "].");
                ++streamErr;
            }
        }
        else
        {
			report.push_back(string("[failed] create error [") + Utility::UnicodeToUTF8(fe.file_name) + "].");
            ++streamErr;
        }
	}

	stringstream ss;
	ss << count << "/" << entries.size() << ", " << streamErr << " for stream error";

	ofstream reportTxt("blacksheepreport.txt", ios::binary);
	if (reportTxt.is_open())
	{
		reportTxt.write(ss.str().c_str(), ss.str().size());
		reportTxt.write("\n-----------------------------------\n", 1);
		for (auto& r : report)
		{
			reportTxt.write(r.c_str(), r.size());
			reportTxt.write("\n", 1);
		}
		reportTxt.close();
	}
	MessageBox(NULL, (string("finish(") + ss.str() + "), see blacksheepreport.txt for detail.").c_str(), szAppName, MB_ICONINFORMATION);

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
    MessageBox(NULL, "UnLoading", szAppName, MB_ICONINFORMATION);
    // Stub使用完毕(必须编写)
    TVPUninitImportStub();

    return S_OK;
}

uint32_t old;
uint32_t addr;
char * func;

extern "C" __declspec(naked) void ProcAddressHookWin7SP1()
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


extern "C" __declspec(naked) void ProcAddressHookWin10()
{
    __asm {
        push ebp
        mov ebp, esp
        mov old, eax
        mov eax, dword ptr[esp + 0x10]
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
        mov esp, ebp
        pop ebp
        mov eax, old
        pop ebp
        retn 0x8
    }
}

void ResumeOtherThread()
{
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
    return;
}

DWORD WINAPI ThreadRoutineWin7SP1(LPVOID)
{
    MessageBox(NULL, "start", szAppName, MB_ICONINFORMATION);

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

    *(uint32_t*)&NewBytes[1] = (uint32_t)ProcAddressHookWin7SP1 - OldAddress - 5;
    SIZE_T W;
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)OldAddress, NewBytes, sizeof(NewBytes), &W);
    assert(W == sizeof(NewBytes));

    ResumeOtherThread();
    return 0;
}

DWORD WINAPI ThreadRoutineWin10(LPVOID)
{
    MessageBox(NULL, "start", szAppName, MB_ICONINFORMATION);

    HMODULE hNtdll = LoadLibrary("kernel32.dll");
    uint32_t OldAddress = (uint32_t)GetProcAddress(hNtdll, "GetProcAddress");
    SIZE_T R;
    char temp[64];
    ReadProcessMemory(GetCurrentProcess(), (LPVOID)OldAddress, temp, sizeof(temp), &R);
    assert(R == sizeof(temp));
    for (int i = 0; i < sizeof(temp) - 4; ++i)
    {
        // pop ebp
        // retn 0x10
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

    *(uint32_t*)&NewBytes[1] = (uint32_t)ProcAddressHookWin10 - OldAddress - 5;
    SIZE_T W;
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)OldAddress, NewBytes, sizeof(NewBytes), &W);
    assert(W == sizeof(NewBytes));


    ResumeOtherThread();
    return 0;
}


#define _WIN32_WINNT_WIN10 0x0a00

typedef NTSTATUS(NTAPI* fnRtlGetVersion)(PRTL_OSVERSIONINFOW lpVersionInformation);

VERSIONHELPERAPI
IsWindowsVersionOrGreater2(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
{
    RTL_OSVERSIONINFOEXW verInfo = { 0 };
    verInfo.dwOSVersionInfoSize = sizeof(verInfo);

    static auto RtlGetVersion = (fnRtlGetVersion)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion");
    
    if (RtlGetVersion != 0 && RtlGetVersion((PRTL_OSVERSIONINFOW)&verInfo) == 0)
    {
        if (verInfo.dwMajorVersion > wMajorVersion)
            return true;
        else if (verInfo.dwMajorVersion < wMajorVersion)
            return false;

        if (verInfo.dwMinorVersion > wMinorVersion)
            return true;
        else if (verInfo.dwMinorVersion < wMinorVersion)
            return false;

        if (verInfo.wServicePackMajor >= wServicePackMajor)
            return true;
    }

    return false;

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

        if (!IsWindows7SP1OrGreater())
        {
            MessageBox(NULL, "You need at least Windows 7 with SP1", "Version Not Supported", MB_OK);
        }

        g_VerWin10 = IsWindowsVersionOrGreater2(HIBYTE(_WIN32_WINNT_WIN10), LOBYTE(_WIN32_WINNT_WIN10), 0);

        if (g_VerWin10)
            CreateThread(NULL, 0, ThreadRoutineWin10, 0, 0, NULL);
        else
            CreateThread(NULL, 0, ThreadRoutineWin7SP1, 0, 0, NULL);
    }
    return 1;
}