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
#include <VersionHelpers.h>
#include "utility/utility.h"
#include "../kirikiri/xp3.h"
#include "ShareMem.h"
#include <Psapi.h>
#include "Detours/detours.h"
#include "log/log.h"

using namespace std;

const char szAppName[] = "BlackSheep";


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

MapObj* g_Map{ nullptr };
HANDLE g_FileMapping{ NULL };
bool g_ITVPFunctionExporterInited{ false };

typedef tTJSBinaryStream* (__fastcall* FN_RealCreateIStream2)(const ttstr&, tjs_uint32);
FN_RealCreateIStream2 fnRealCreateIStream2;
typedef FARPROC (WINAPI* FN_GetProcAddress)(HMODULE hModule, LPCSTR lpProcName);
FN_GetProcAddress pfnGetProcAddress;
typedef HRESULT (__stdcall* FN_V2Link)(iTVPFunctionExporter* exporter);
FN_V2Link pfnV2Link;



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
    wstring drv, dir, file, ext;
    Utility::SplitPath(out_dir, drv, dir, file, ext);
    out_dir = drv + dir + L"[passive]";
    return out_dir;
  }
}

wstring ToString(const ttstr& ts) {
  // 调用得比 TVPInitImportStub 早，还不能用 ttstr::c_str()
  tTJSVariantString_S* p = *(tTJSVariantString_S**)&ts;
  if (p && p->LongString)
    return wstring(p->LongString, p->LongString + p->Length);
  else if (p && p->Length <= TJS_VS_SHORT_LEN)
    return wstring(p->ShortString, p->ShortString + p->Length);
  else
    return wstring();
}


const constexpr std::array<char, 76> kPatternTVPCreateStream = {
  '\x55', '\x8B', '\xEC', '\x6A', '\xFF', '\x68', '?','?','?','?', '\x64', '\xA1', '\x00', '\x00', '\x00', '\x00',
  '\x50', '\x83', '\xEC', '\x5C', '\x53', '\x56', '\x57', '\xA1', '?','?','?','?', '\x33', '\xC5', '\x50', '\x8D',
  '\x45', '\xF4', '\x64', '\xA3', '\x00', '\x00', '\x00', '\x00', '\x89', '\x65', '\xF0', '\x89', '\x4D', '\xEC',
  '\xC7', '\x45', '\xFC', '\x00', '\x00', '\x00', '\x00', '\xE8', '\x36', '\xFD', '\xFF', '\xFF', '\x8B', '\x4D',
  '\xF4', '\x64', '\x89', '\x0D', '\x00', '\x00', '\x00', '\x00', '\x59', '\x5F', '\x5E', '\x5B', '\x8B', '\xE5',
  '\x5D', '\xC3'
};

vector<char> TryDecode(const vector<char>& buffer) {
  if (buffer.size() < 5)
    return buffer;
  if (buffer[0] != '\xfe' || buffer[1] != '\xfe' || buffer[3] != '\xff' || buffer[4] != '\xfe')
    return buffer;
  const char type = buffer[2];
  switch (type) {
  case 1: {
    vector<char> result(buffer.begin() + 5, buffer.end());
    for (size_t i = 0; i < result.size(); ++i) {
      result[i] = ((result[i] & 0xaaaaaaaa) >> 1) | ((result[i] & 0x55555555) << 1);
    }
    return result;
  }
  }
  return buffer;
}

void DumpResource(tTJSBinaryStream* ibs, const ttstr& ts, tjs_uint32 flag) {
  const constexpr std::array<wchar_t*, 5> kFilenamePattern = {
    LR"((file://./)?.+?\.xp3>(.+))",
    LR"(arc://./(.+))",
    LR"(archive://./(.+))",
    LR"(bres://./(.+))",
    LR"((^[_a-zA-Z0-9]+\.[a-zA-Z0-9]+$))",
  };

  if (!ibs)
    return;
  wstring filename;
  bool match{ false };
  for (auto& p : kFilenamePattern) {
    std::wregex expr(p, std::regex_constants::icase);
    std::wsmatch result;
    wstring tmp = ToString(ts);
    if (std::regex_match(tmp, result, expr) && !result.empty()) {
      filename = result[result.size() - 1].str();
      match = true;
      break;
    }
  }
  if (!match) {
    LOG_DEBUG("skip file due to filter: " << Utility::UnicodeToUTF8(ToString(ts)));
    return;
  }
  wstring save_dir;
  LOG_INFO("saving: " << Utility::UnicodeToUTF8(ToString(ts)));
  tjs_uint64 saved_pos = ibs->Seek(0, TJS_BS_SEEK_CUR);
  std::vector<char> buffer((size_t)ibs->GetSize(), 0);
  ibs->Seek(0, TJS_BS_SEEK_SET);
  bool success = buffer.size() > 0 && ibs->Read(buffer.data(), buffer.size()) == buffer.size();
  if (success) {
    buffer = TryDecode(buffer);
    save_dir = GetSaveFolder(true);
    CreateDirectoryW(save_dir.c_str(), 0);
    success = (ERROR_SUCCESS == EncryptedXP3::SplitFileNameAndSave(save_dir, filename, buffer));
  }
  ibs->Seek(saved_pos, TJS_BS_SEEK_SET);
  if (success)
    LOG_INFO("saved to: " << Utility::UnicodeToUTF8(save_dir) << "\\" << Utility::UnicodeToUTF8(filename));
  else
    LOG_ERROR("saved " << Utility::UnicodeToUTF8(filename) << " failed");
}

tTJSBinaryStream* __fastcall DetouredCreateIStream2(const ttstr& ts, tjs_uint32 flag) {
  tTJSBinaryStream* ibs = fnRealCreateIStream2(ts, flag);
  if (g_Map->PassiveMode) {
    DumpResource(ibs, ts, flag);
  }
  return ibs;
}

void HandleXp3File(const wstring& xp3Path) {
  wstring drv, dir, file, ext;
  Utility::SplitPath(xp3Path, drv, dir, file, ext);
  wstring outDir = drv + dir + L"[extract] " + file + ext;
  CreateDirectoryW(outDir.c_str(), 0);

  LOG_INFO("process file: " + Utility::UnicodeToUTF8(xp3Path));

  EncryptedXP3* xp3 = new EncryptedXP3();
  bool b = xp3->Open(xp3Path);
  if (!b) {
    LOG_ERROR(Utility::UnicodeToUTF8(xp3Path) << " open failed.");
    return;
  }

  vector<file_entry> entries = xp3->ExtractEntries(xp3->GetPlainIndexBytes());
  int count = 0, streamErr = 0;
  for (const file_entry& fe : entries) {
    ttstr name(xp3->FormatFileNameForIStream(fe).c_str());
    IStream* stream = TVPCreateIStream(name, 0);
    if (!stream) {
      LOG_ERROR("failed creating stream: " << Utility::UnicodeToUTF8(fe.file_name));
      continue;
    }
    STATSTG state;
    DWORD flag{ 1 };
    if (FAILED(stream->Stat(&state, flag))) {
      LOG_ERROR("failed reading stream state: " << Utility::UnicodeToUTF8(fe.file_name));
      continue;
    }
    size_t fileSize = (size_t)state.cbSize.QuadPart;
    LPVOID buf = VirtualAlloc(NULL, fileSize, MEM_COMMIT, PAGE_READWRITE);
    ULONG R{ 0 };
    if (SUCCEEDED(stream->Read(buf, fileSize, &R)) && R == fileSize) {
      assert(R == fileSize);
      if (0 == EncryptedXP3::SplitFileNameAndSave(outDir, fe.file_name, vector<char>((char*)buf, (char*)buf + fileSize))) {
        LOG_INFO(Utility::UnicodeToUTF8(fe.file_name) << " saved");
        ++count;
      }
      else {
        LOG_ERROR(Utility::UnicodeToUTF8(fe.file_name) << " can not save");
      }
    }
    else {
      LOG_ERROR("failed reading stream bytes: " << Utility::UnicodeToUTF8(fe.file_name)
        << ", to read: " << fileSize << ", actual read: " << R);
    }
    VirtualFree(buf, 0, MEM_RELEASE);
  }

  LOG_INFO("success: " << count << "/" << entries.size());
  LOG_INFO("process file: " + Utility::UnicodeToUTF8(xp3Path) << " finished");
}

DWORD WINAPI ExtractXP3(LPVOID) {
  MessageBox(NULL, "Begin extracting, wait a minute....", szAppName, MB_ICONINFORMATION);

  while (!g_ITVPFunctionExporterInited)
    Sleep(1);
  assert(g_Map->Count <= _countof(MapObj::Path));
  for (int i = 0; i < g_Map->Count; ++i) {
    HandleXp3File(wstring(g_Map->Path[i]));
  }

  MessageBox(NULL, "finish extracting, see blacksheep.report for details.", szAppName, MB_ICONINFORMATION);
  return 0;
}

HRESULT __stdcall DetouredV2Link(iTVPFunctionExporter* exporter) {
  auto result = pfnV2Link(exporter);
  TVPInitImportStub(exporter);
  g_ITVPFunctionExporterInited = true;
  LOG_INFO("ITVPFunctionExporter initialized");
  if (!g_Map->PassiveMode) {
    CreateThread(0, 0, ExtractXP3, 0, 0, 0);
  }
  return result;
}

FARPROC WINAPI DetouredGetProcAddress(HMODULE hModule, LPCSTR lpProcName) {
  auto fn = pfnGetProcAddress(hModule, lpProcName);
  if (string(lpProcName) == "V2Link") {
    pfnV2Link = (FN_V2Link)fn;
    LOG_INFO("add hook for V2Link");
    Utility::AddHook(&pfnV2Link, DetouredV2Link);
    Utility::RemoveHook(&pfnGetProcAddress, DetouredGetProcAddress);
  }
  return fn;
}

DWORD WINAPI Entrance(LPVOID)
{
  MODULEINFO mi = { 0 };
  GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(NULL), &mi, sizeof(mi));
  fnRealCreateIStream2 = (FN_RealCreateIStream2)Utility::SearchSequence((uint8_t*)GetModuleHandleA(NULL), mi.SizeOfImage, { kPatternTVPCreateStream.begin(), kPatternTVPCreateStream.end() });
  LOG_INFO("found entrance of <RealCreateIStream>: " << fnRealCreateIStream2);
  Utility::AddHook(&fnRealCreateIStream2, DetouredCreateIStream2);

  pfnGetProcAddress = GetProcAddress;
  Utility::AddHook(&pfnGetProcAddress, DetouredGetProcAddress);

  Utility::ResumeOtherThread();
  return 0;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD Reason, LPVOID lpReserved)
{
  if (Reason == DLL_PROCESS_ATTACH)
  {
    MessageBox(NULL, "start", szAppName, MB_ICONINFORMATION);
    Log::Init("blacksheep.report");
    LOG_INFO("======================= log start =======================");
    g_FileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, 0, 4096, "BlackSheep{13E025E2-B7AA-4141-9B4E-402CCB3C4F33}");
    assert(g_FileMapping != NULL);
    g_Map = (MapObj*)MapViewOfFile(g_FileMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(MapObj));
    assert(g_Map);
    g_Map->Access = TRUE;
    LOG_INFO("passive mode: " << g_Map->PassiveMode);
    CreateThread(NULL, 0, Entrance, 0, 0, NULL);
  }
  return 1;
}

extern "C" __declspec(dllexport) HRESULT _stdcall V2Link(iTVPFunctionExporter *exporter) {
  return S_OK;
}

extern "C" __declspec(dllexport) HRESULT _stdcall V2Unlink() {
  return S_OK;
}