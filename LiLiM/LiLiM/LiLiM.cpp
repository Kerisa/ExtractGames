#include <array>
#include <strsafe.h>
#include <cassert>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <Windows.h>
#include "LiLiM.h"
#include "picture.h"

using namespace std;

static const int MAXPATH = 350;

namespace {

std::string WCSToANSI(const std::wstring& wcs)
{
  int size = WideCharToMultiByte(CP_ACP, 0, wcs.data(), -1, 0, 0, 0, 0);
  std::vector<char> mbs(size + 1);
  WideCharToMultiByte(CP_ACP, 0, wcs.data(), -1, mbs.data(), mbs.size(), NULL, NULL);
  return mbs.data();
}

std::string JPToANSI(const std::string& str)
{
  int size = MultiByteToWideChar(932, 0, str.c_str(), -1, 0, 0);
  std::vector<wchar_t> wcs(size + 1);
  MultiByteToWideChar(932, 0, str.c_str(), -1, wcs.data(), wcs.size());

  size = WideCharToMultiByte(CP_ACP, 0, wcs.data(), -1, 0, 0, 0, 0);
  std::vector<char> mbs(size + 1);
  WideCharToMultiByte(CP_ACP, 0, wcs.data(), -1, mbs.data(), mbs.size(), NULL, NULL);
  return mbs.data();
}

std::wstring JPToWCS(const std::string& str)
{
  int size = MultiByteToWideChar(932, 0, str.c_str(), -1, 0, 0);
  std::vector<wchar_t> wcs(size + 1);
  MultiByteToWideChar(932, 0, str.c_str(), -1, wcs.data(), wcs.size());
  return wcs.data();
}

int SplitFileNameAndSave(const string& cur_dir, const string& file_name, const vector<char>& unpackData)
{
  DWORD ByteWrite;
  string buf;

  buf = cur_dir + "\\" + file_name;
  for (char& c : buf)
    if (c == '/')
      c = '\\';
  for (size_t p = buf.find('\\'); p != string::npos; p = buf.find('\\', p + 1)) {
    CreateDirectoryA(buf.substr(0, p).c_str(), 0);
  }

  HANDLE hFile{ INVALID_HANDLE_VALUE };
  int ret{ -1 };
  hFile = CreateFileA(buf.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
  if (hFile != INVALID_HANDLE_VALUE) {
    BOOL success = WriteFile(hFile, unpackData.data(), unpackData.size(), &ByteWrite, NULL);
    ret = (success && ByteWrite == unpackData.size()) ? ERROR_SUCCESS : GetLastError();
  }
  else {
    ret = GetLastError();
  }
  CloseHandle(hFile);
  return ret;
}

}

namespace abm {

  bool DecompABM(uint8_t* dst, size_t dst_size, const uint8_t* src, size_t src_size)
  {
    size_t src_ptr = 0;
    size_t dst_ptr = 0;
    uint32_t alpha_count = 0;

    while (dst_ptr < dst_size)
    {
      const uint8_t current_src = src[src_ptr++];

      switch (current_src)
      {
      case 0:
      {
        const uint32_t length = src[src_ptr++];

        for (size_t i = 0; i < length; i++)
        {
          dst[dst_ptr++] = 0;

          alpha_count++;

          if (alpha_count == 3)
          {
            dst[dst_ptr++] = 0;
            alpha_count = 0;
          }
        }

        break;
      }

      case 255:
      {
        const uint32_t length = src[src_ptr++];

        for (size_t i = 0; i < length; i++)
        {
          dst[dst_ptr++] = src[src_ptr++];

          alpha_count++;

          if (alpha_count == 3)
          {
            dst[dst_ptr++] = 0xFF;
            alpha_count = 0;
          }
        }
        break;
      }

      default: // Other
        dst[dst_ptr++] = src[src_ptr++];
        alpha_count++;

        if (alpha_count == 3)
        {
          dst[dst_ptr++] = current_src;
          alpha_count = 0;
        }
      }
    }

    return true;
  }

  bool DecodeABM0(const wstring& file_name, const std::vector<uint8_t>& src) {
    const auto* bitmap_file_header = (const BITMAPFILEHEADER*)&src[0];
    const auto* bitmap_info_header = (const BITMAPINFOHEADER*)&src[14];

    std::vector<uint8_t> dst;
    uint32_t             dst_size;

    switch (bitmap_info_header->biBitCount)
    {
    case 1: // Multi-frame
    {
      wstring last_name = file_name;
      const uint32_t frames = *(const uint32_t*)&src[58];
      const uint32_t data_offset = *(const uint32_t*)&src[66];

      dst_size = bitmap_info_header->biWidth * bitmap_info_header->biHeight * 4;
      dst.resize(dst_size);

      // Multiple files
      if (frames >= 2)
      {
        last_name.append(L"_000");
      }

      uint32_t src_ptr = data_offset;
      for (size_t i = 0; i < dst.size(); i += 4)
      {
        dst[i + 0] = src[src_ptr++];
        dst[i + 1] = src[src_ptr++];
        dst[i + 2] = src[src_ptr++];
        dst[i + 3] = 0xFF;
      }

      Alisa::Image img;
      Alisa::ImageInfo iinfo;
      iinfo.Width = bitmap_info_header->biWidth;
      iinfo.Height = bitmap_info_header->biHeight;
      iinfo.Component = 4;
      iinfo.FrameCount = 1;
      img.NewImage(iinfo);
      img.WritePixels(dst, true, true, false);
      img.SaveTo(last_name, Alisa::E_ImageType_Bmp);

      for (uint32_t i = 1; i < frames; i++)
      {
        const uint32_t frame_offset = *(const uint32_t*)&src[70 + (i - 1) * 4];
        last_name = file_name + (wstringstream() << setw(3) << setfill(L'0') << i).str();

        std::fill(dst.begin(), dst.end(), 0);
        DecompABM(dst.data(), dst.size(), &src[frame_offset], src.size() - frame_offset);

        img.NewImage(iinfo);
        img.WritePixels(dst, true, true, false);
        img.SaveTo(last_name, Alisa::E_ImageType_Bmp);
      }
      break;
    }

    case 32: { // 32bit
      dst_size = bitmap_info_header->biWidth * bitmap_info_header->biHeight * 4;
      dst.resize(dst_size);

      DecompABM(dst.data(), dst.size(), &src[54], src.size() - 54);

      Alisa::Image img;
      Alisa::ImageInfo iinfo;
      iinfo.Width = bitmap_info_header->biWidth;
      iinfo.Height = bitmap_info_header->biHeight;
      iinfo.Component = 4;
      iinfo.FrameCount = 1;
      img.NewImage(iinfo);
      img.WritePixels(dst, true, true, false);
      img.SaveTo(file_name, Alisa::E_ImageType_Bmp);
      break;
    }
    default:
      return false;
    }

    return true;
  }
}

namespace huffman {
  std::array<int, 0x200> dword_4BCB38;
  std::array<int, 0x200> dword_4BD338;
  int dword_4C358C;
  int dword_4C3590;
  int dword_4BD334;
  int dword_4BCB34;
  int dword_4BCB30;

  template<class T> char __SETS__(T x)
  {
    if (sizeof(T) == 1)
      return char(x) < 0;
    if (sizeof(T) == 2)
      return short(x) < 0;
    if (sizeof(T) == 4)
      return int(x) < 0;
    return long long(x) < 0;
  }

  // overflow flag of subtraction (x-y)
  template<class T, class U> char __OFSUB__(T x, U y)
  {
    if (sizeof(T) < sizeof(U))
    {
      U x2 = x;
      char sx = __SETS__(x2);
      return (sx ^ __SETS__(y)) & (sx ^ __SETS__(x2 - y));
    }
    else
    {
      T y2 = y;
      char sx = __SETS__(x);
      return (sx ^ __SETS__(y2)) & (sx ^ __SETS__(x - y2));
    }
  }

  int sub_419CE0()
  {
    signed int v0; // ecx
    unsigned int v1; // eax
    signed int v2; // edx
    int v3; // edi
    int v4; // esi
    int v5; // ebx
    unsigned int v6; // eax
    int v7; // esi
    bool v8; // sf
    unsigned __int8 v9; // of
    int result; // eax

    if (--dword_4C358C < 0)
    {
      dword_4C3590 = *(unsigned __int8*)dword_4BCB30;
      dword_4C358C = 7;
      ++dword_4BCB30;
      v6 = (unsigned int)dword_4C3590 >> 7;
    }
    else
    {
      v6 = (unsigned int)dword_4C3590 >> dword_4C358C;
    }
    if (v6 & 1)
    {
      v7 = dword_4BD334;
      v9 = __OFSUB__(dword_4BD334, 511);
      v8 = dword_4BD334++ - 511 < 0;
      if (v8 ^ v9)
      {
        dword_4BD338[v7] = sub_419CE0();
        dword_4BCB38[v7] = sub_419CE0();
        result = v7;
      }
      else
      {
        result = 0;
      }
    }
    else
    {
      v0 = dword_4C358C;
      v1 = dword_4C3590;
      v2 = 8;
      v3 = 0;
      if (dword_4C358C < 8)
      {
        v4 = dword_4BCB30;
        do
        {
          v2 -= v0;
          ++v4;
          v5 = v1 & ((1 << v0) - 1);
          v1 = *(unsigned __int8*)(v4 - 1);
          v0 = 8;
          v3 |= v5 << v2;
        } while (v2 > 8);
        dword_4BCB30 = v4;
        dword_4C3590 = v1;
      }
      dword_4C358C = v0 - v2;
      result = v3 | ((1 << v2) - 1) & (v1 >> (v0 - v2));
    }
    return result;
  }

  unsigned int decode(unsigned __int16* a1, int a2)
  {
    BYTE* v2; // ebp
    unsigned __int16* v3; // eax
    int v4; // ebx
    int v5; // ecx
    int v6; // edx
    unsigned int v7; // ebx
    int v8; // edx
    unsigned int result; // eax
    signed int v10; // ecx
    unsigned int v11; // esi
    unsigned __int8* v12; // edi
    int v13; // eax
    unsigned int v14; // edx
    unsigned int v15; // edx
    bool v16; // zf
    int v17; // [esp+10h] [ebp-4h]
    unsigned int v18; // [esp+18h] [ebp+4h]

    v2 = (BYTE*)a2;
    memset(dword_4BD338.data(), 0, sizeof(dword_4BD338));
    memset(dword_4BCB38.data(), 0, sizeof(dword_4BCB38));
    v3 = a1 + 1;
    v4 = *a1;
    v5 = *((unsigned __int8*)a1 + 3);
    v6 = *((unsigned __int8*)a1 + 2) << 16;
    dword_4C358C = 0;
    dword_4C3590 = 0;
    dword_4BD334 = 256;
    dword_4BCB34 = a2;
    v7 = (v5 << 24) | v6 | v4;
    dword_4BCB30 = (int)(v3 + 1);
    v8 = sub_419CE0();
    result = 0;
    v17 = v8;
    if (v7 > 0)
    {
      v10 = dword_4C358C;
      v11 = dword_4C3590;
      v12 = (unsigned __int8*)dword_4BCB30;
      v18 = v7;
      do
      {
        v13 = v8;
        if (v8 >= 256)
        {
          do
          {
            if (--v10 < 0)
            {
              v11 = *v12;
              v15 = *v12++;
              v10 = 7;
              v14 = v15 >> 7;
            }
            else
            {
              v14 = v11 >> v10;
            }
            if (v14 & 1)
              v13 = dword_4BCB38[v13];
            else
              v13 = dword_4BD338[v13];
          } while (v13 >= 256);
          v8 = v17;
          dword_4BCB30 = (int)v12;
          dword_4C3590 = v11;
          dword_4C358C = v10;
        }
        *v2++ = v13;
        v16 = v18-- == 1;
        dword_4BCB34 = (int)v2;
      } while (!v16);
      result = v7;
    }
    //return result;
    return dword_4BCB34 - a2;
  }
}

int Entrance(const wchar_t* PackName, const wchar_t* CurDir)
{
  DWORD R, FileSaved = 0;
  PACKHEADER ph;
  IDX idx;
  wchar_t MsgBuf[MAXPATH];

  HANDLE hFile = CreateFile(PackName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if (hFile == INVALID_HANDLE_VALUE)
  {
    StringCchPrintf(MsgBuf, MAXPATH, L"文件无法打开 %s\r\n", PackName);
    AppendMsg(MsgBuf);
    return -1;
  }


  ReadFile(hFile, &ph, sizeof(ph), &R, 0);
  ph.FileNum /= sizeof(IDX);

  for (DWORD i = 0; i < ph.FileNum; ++i)
  {
    SetFilePointer(hFile, 0x111 + i * sizeof(idx), 0, FILE_BEGIN);
    ReadFile(hFile, &idx, sizeof(idx), &R, 0);

    vector<char> data(idx.FileSize);
    // 明文，抽出来直接保存
    LARGE_INTEGER li;
    li.QuadPart = ph.FileDataOffset + idx.FileOffset;
    auto ret = SetFilePointerEx(hFile, li, 0, FILE_BEGIN);
    assert(ret);
    ReadFile(hFile, data.data(), idx.FileSize, &R, 0);

    bool already_save = false;
    string file_name(idx.FileName);
    if (file_name.find(".scr") != string::npos) {
      vector<char> plain(idx.FileSize * 8);
      int offset = huffman::decode((unsigned short*)data.data(), (int)plain.data());
      plain.erase(plain.begin() + offset, plain.end());
      data.swap(plain);
    }
    else if (file_name.find(".abm") != string::npos) {
      file_name.replace(file_name.begin() + file_name.find(".abm"), file_name.end(), ".bmp");
      already_save = abm::DecodeABM0(wstring(CurDir) + L'\\' + JPToWCS(file_name), vector<uint8_t>(data.begin(), data.begin() + data.size()));
    }
    else if (file_name.find(".cmp") != string::npos) {
      file_name.replace(file_name.begin() + file_name.find(".cmp"), file_name.end(), ".bmp");
      vector<char> plain(idx.FileSize * 8);
      int offset = huffman::decode((unsigned short*)data.data(), (int)plain.data());
      plain.erase(plain.begin() + offset, plain.end());
      data.swap(plain);
      already_save = abm::DecodeABM0(wstring(CurDir) + L'\\' + JPToWCS(file_name), vector<uint8_t>(data.begin(), data.begin() + data.size()));
    }
    else if (file_name.find(".msk") != string::npos) {
      file_name.replace(file_name.begin() + file_name.find(".msk"), file_name.end(), ".bmp");
    }

    wstringstream wss;
    int ret2 = 0;
    if (!already_save)
      ret2 = SplitFileNameAndSave(WCSToANSI(CurDir), JPToANSI(file_name), data);
    if (ret2 == 0) {
      ++FileSaved;
      wss << L"[已保存] " << JPToWCS(file_name) << "\r\n";
    }
    else {
      wss << L"[无法保存] " << JPToWCS(file_name) << ", 错误码 " << ret2 << "\r\n";
    }
    AppendMsg(wss.str().c_str());
  }

  if (FileSaved == ph.FileNum)
  {
    StringCchPrintf(MsgBuf, MAXPATH, L"[提取完毕(%d/%d)] %s\r\n", FileSaved, FileSaved, PackName);
    AppendMsg(MsgBuf);
  }
  else {
    StringCchPrintf(MsgBuf, MAXPATH, L"[提取完毕(%d/%d)] %s\r\n有%d个文件提取失败",
      FileSaved, ph.FileNum, PackName, ph.FileNum - FileSaved);
    MessageBox(0, MsgBuf, 0, MB_ICONWARNING);
  }

  return 0;
}