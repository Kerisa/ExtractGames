// BlackSheep.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <array>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <regex>
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
#include "ShareMem.h"
#include <Psapi.h>

using namespace std;

const char szAppName[] = "BlackSheep";

MapObj* g_Map;



class tTJSBinaryStream
{
public:
  //-- must implement
  virtual tjs_uint64 TJS_INTF_METHOD Seek(tjs_int64 offset, tjs_int whence) = 0;
  /* if error, position is not changed */

//-- optionally to implement
  virtual tjs_uint TJS_INTF_METHOD Read(void* buffer, tjs_uint read_size) = 0;
  /* returns actually read size */

  virtual tjs_uint TJS_INTF_METHOD Write(const void* buffer, tjs_uint write_size) = 0;
  /* returns actually written size */

  virtual void TJS_INTF_METHOD SetEndOfStorage();
  // the default behavior is raising a exception
  /* if error, raises exception */

//-- should re-implement for higher performance
  virtual tjs_uint64 TJS_INTF_METHOD GetSize() = 0;

  virtual ~tTJSBinaryStream() { ; }
};

HANDLE hFileMapping;
uint32_t Finish = 0;
uint32_t OldV2Link = 0;
iTVPFunctionExporter *exporter;
uint32_t retAddr;
uint32_t temp;
bool g_VerWin10;
typedef tTJSBinaryStream* (__fastcall* FN_RealCreateIStream2)(const ttstr&, tjs_uint32);
FN_RealCreateIStream2 fnRealCreateIStream2;

std::wstring GetSaveFolder(bool passive_save_dir) {
  wstring out_dir;
  if (g_Map && g_Map->Path[0][0] != 0) {
    wstring drv, dir, file, ext;
    Utility::SplitPath(g_Map->Path[0], drv, dir, file, ext);
    if (passive_save_dir) {
      out_dir = drv + dir + L"[passive]";
    }
    else {
      out_dir = drv + dir + L"[extract] " + file + ext;
    }
    return out_dir;
  }
  else {
    // use .exe path
    out_dir.resize(65535);
    DWORD size = GetModuleFileNameW(GetModuleHandle(NULL), &out_dir[0], out_dir.size());
    assert((size < out_dir.size()) && (GetLastError() == 0) && "get module file name failed");
    out_dir.resize(size);
    return out_dir;
  }
}

wstring ToString(const ttstr& ts) {
  // 调用得比 TVPInitImportStub，还不能用 ttstr::c_str()
  tTJSVariantString_S* p = *(tTJSVariantString_S**)&ts;
  if (p && p->LongString)
    return wstring(p->LongString, p->LongString + p->Length);
  else if (p && p->Length <= TJS_VS_SHORT_LEN)
    return wstring(p->ShortString, p->ShortString + p->Length);
  else
    return wstring();
}


const constexpr std::array<char, 76> PatternTVPCreateStream = {
  '\x55', '\x8B', '\xEC', '\x6A', '\xFF', '\x68', '?','?','?','?', '\x64', '\xA1', '\x00', '\x00', '\x00', '\x00',
  '\x50', '\x83', '\xEC', '\x5C', '\x53', '\x56', '\x57', '\xA1', '?','?','?','?', '\x33', '\xC5', '\x50', '\x8D',
  '\x45', '\xF4', '\x64', '\xA3', '\x00', '\x00', '\x00', '\x00', '\x89', '\x65', '\xF0', '\x89', '\x4D', '\xEC',
  '\xC7', '\x45', '\xFC', '\x00', '\x00', '\x00', '\x00', '\xE8', '\x36', '\xFD', '\xFF', '\xFF', '\x8B', '\x4D',
  '\xF4', '\x64', '\x89', '\x0D', '\x00', '\x00', '\x00', '\x00', '\x59', '\x5F', '\x5E', '\x5B', '\x8B', '\xE5',
  '\x5D', '\xC3'
};

std::string getTimeStr() {
  std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  std::string s(30, '\0');
  tm tmmmmm;
  localtime_s(&tmmmmm, &now);
  std::strftime(&s[0], s.size(), "%Y-%m-%d %H:%M:%S", &tmmmmm);
  return s.c_str();
}

void Sub(tTJSBinaryStream* ibs, const ttstr& ts, tjs_uint32 flag) {
  const constexpr std::array<wchar_t*, 3> FilenamePattern = {
    LR"((file://./)?.+?\.xp3>(.+))",
    LR"(arc://./(.+))",
    LR"(archive://./(.+))"
  };

  if (ibs) {
    ofstream reportTxt("passive_dump_report.txt", ios::binary | ios::app);
    wstring filename;
    bool match{ false };
    for (auto& p : FilenamePattern) {
      std::wregex expr(p, std::regex_constants::icase);
      std::wsmatch result;
      wstring tmp = ToString(ts);
      if (std::regex_match(tmp, result, expr) && !result.empty()) {
        filename = result[result.size() - 1].str();
        match = true;
      }
      else {
        //reportTxt.write(("[skip] " + Utility::UnicodeToUTF8(ts.c_str())), )
      }
    }
    if (match) {
      //tTJSBinaryStream* ibs = TVPCreateBinaryStreamAdapter(is);
      tjs_uint64 saved_pos = ibs->Seek(0, TJS_BS_SEEK_CUR);
      std::vector<char> buffer(ibs->GetSize(), 0);
      ibs->Seek(0, TJS_BS_SEEK_SET);
      if (buffer.size() > 0 && ibs->Read(buffer.data(), buffer.size()) == buffer.size()) {
        wstring save_dir = GetSaveFolder(true);
        CreateDirectoryW(save_dir.c_str(), 0);
        int ret = EncryptedXP3::SplitFileNameAndSave(save_dir, filename, buffer);
      }
      ibs->Seek(saved_pos, TJS_BS_SEEK_SET);
    }
  }
}

tTJSBinaryStream* __fastcall DetouredCreateIStream2(const ttstr& ts, tjs_uint32 flag) {

  static bool first{ true };
  if (first) {
    MessageBox(NULL, "first time", szAppName, MB_ICONINFORMATION);
    first = false;
  }

  tTJSBinaryStream* ibs = fnRealCreateIStream2(ts, flag);
  Sub(ibs, ts, flag);
  return ibs;
}


char original_template[] = {
  // <oringal bytes>
  // jmp <original + length of bytes>
  '\x00', '\x00', '\x00', '\x00', '\x90',
  '\xe9', '\xff', '\xff', '\xff', '\xff',
  '\xcc',
};

char detoured_template_v2[] = {
  // call <real_detoured_function>  ; replace this 4 bytes with proxy funtion
  // jmp <original_template>
  '\xe8', '\xff', '\xff', '\xff', '\xff',
  '\xc3',//'\xe9', '\xff', '\xff', '\xff', '\xff',
  '\xcc',
};

void InlineHook(FN_RealCreateIStream2* poriginal, FN_RealCreateIStream2 detoured) {

  FN_RealCreateIStream2 original = *poriginal;
  char orginal_bytes[5];
  SIZE_T R{ 0 }, W{ 0 };
  ReadProcessMemory(GetCurrentProcess(), original, orginal_bytes, sizeof(orginal_bytes), &R);

  DWORD dummy;
  VirtualProtect(detoured_template_v2, sizeof(detoured_template_v2), PAGE_EXECUTE_READWRITE, &dummy);
  VirtualProtect(original_template, sizeof(original_template), PAGE_EXECUTE_READWRITE, &dummy);

  *(uint32_t*)&detoured_template_v2[1] = (uint32_t)detoured - (uint32_t)&detoured_template_v2[1] - 4;
  //*(uint32_t*)&detoured_template_v2[6] = (uint32_t)original_template - (uint32_t)&detoured_template_v2[6] - 4;
  //memcpy(&detoured_template[16], orginal_bytes, sizeof(orginal_bytes));
  //*(uint32_t*)&detoured_template[23] = (uint32_t)original + sizeof(orginal_bytes);
  memcpy(&original_template[0], orginal_bytes, sizeof(orginal_bytes));
  *(uint32_t*)&original_template[6] = (uint32_t)original + sizeof(orginal_bytes) - (uint32_t)&original_template[6] - 4;
  char replaced_entrance[sizeof(orginal_bytes)]{ '\xE9' };  // jmp
  *(uint32_t*)&replaced_entrance[1] = (uint32_t)detoured_template_v2 - (uint32_t)original - 5;   // need make a copy for each detour function
  //replaced_entrance[5] = '\x90';
  WriteProcessMemory(GetCurrentProcess(), original, replaced_entrance, sizeof(replaced_entrance), &W);
  *poriginal = (FN_RealCreateIStream2)&original_template;
}

void HandleXp3File(const wchar_t* xp3Path, vector<string>& report)
{

    wstring drv, dir, file, ext;
    Utility::SplitPath(xp3Path, drv, dir, file, ext);
    wstring outDir = drv + dir + L"[extract] " + file + ext;
    CreateDirectoryW(outDir.c_str(), 0);

    report.push_back("process file [" + Utility::UnicodeToUTF8(xp3Path) + "]:");

    EncryptedXP3 *xp3 = new EncryptedXP3();
    bool b = xp3->Open(xp3Path);
    if (!b)
    {
        stringstream ss;
        ss << "xp3 [" << xp3Path << "] open failed.";
        report.push_back(ss.str());
        MessageBox(NULL, ss.str().c_str(), szAppName, MB_ICONWARNING);
        return;
    }

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
                }
                else
                {
                    report.push_back(string("[failed] reading [") + Utility::UnicodeToUTF8(fe.file_name) + "].");
                    ++streamErr;
                }
                VirtualFree(buf, 0, MEM_RELEASE);
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
    report.push_back(ss.str());
    report.push_back("-----------------------------------");
    report.push_back("");
}

DWORD WINAPI Worker(LPVOID)
{
    //MessageBox(NULL, "seems OK, begin extracting, wait a minute....", szAppName, MB_ICONINFORMATION);

    vector<string> report;
    while (1)
    {
      Sleep(1);
    }
    assert(g_Map->Count <= _countof(MapObj::Path));
    for (int i = 0; i < g_Map->Count; ++i)
    {
        HandleXp3File(g_Map->Path[i], report);
    }

    ofstream reportTxt("blacksheepreport.txt", ios::binary);
    if (reportTxt.is_open())
    {
        for (auto& r : report)
        {
            reportTxt.write(r.c_str(), r.size());
            reportTxt.write("\n", 1);
        }
        reportTxt.close();
    }
    MessageBox(NULL, "finish process, see blacksheepreport.txt for detail.", szAppName, MB_ICONINFORMATION);

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

uint8_t* SearchSequence(uint8_t* search_start, uint32_t search_length, const vector<char>& pattern) {
  
  for (uint8_t* search_end = search_start + search_length; search_start < search_end - pattern.size(); ++search_start)
  {
    bool found = std::equal(search_start, search_start + pattern.size(), pattern.begin(), pattern.end(), [](char L, char R) {
      return L == R || R == '?';
    });
    if (found)
      return search_start;
  }

  return nullptr;
}

DWORD WINAPI ThreadRoutineWin10(LPVOID)
{
    MessageBox(NULL, "start", szAppName, MB_ICONINFORMATION);

    MODULEINFO mi = { 0 };
    GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(NULL), &mi, sizeof(mi));

    fnRealCreateIStream2 = (FN_RealCreateIStream2)SearchSequence((uint8_t*)GetModuleHandleA(NULL), mi.SizeOfImage, { PatternTVPCreateStream.begin(), PatternTVPCreateStream.end() });
    InlineHook(&fnRealCreateIStream2, DetouredCreateIStream2);

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


//#define _WIN32_WINNT_WIN10 0x0a00

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
        g_Map = (MapObj*)MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(MapObj));
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