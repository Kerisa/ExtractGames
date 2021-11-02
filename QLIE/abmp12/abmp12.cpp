#include <algorithm>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <fstream>
#include <sstream>
#include <vector>
#include <windows.h>

using namespace std;

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

string MakeFileName(const string& curdir, const string& name, int idx, const string& ext) {
  string dir = curdir;
  if (!dir.empty() && dir.back() == '\\')
    dir.pop_back();

  stringstream ss;
  ss << "@" << setw(3) << setfill('0') << to_string(idx);
  return dir + "\\" + name + ss.str() + ext;
}

bool SaveToFile(const string& name, const vector<char>& data) {
  assert(!data.empty());
  string copy;
  transform(name.begin(), name.end(), inserter(copy, copy.end()), [](auto ch) {
    switch (ch) {
    case '*': case '?': case '<': case '>': case '|': case '"': case '/':
      return '_';
    default:
      return ch;
    }
  });
  ofstream out_file(copy, ios::binary);
  assert(out_file.is_open());
  out_file.write(data.data(), data.size());
  out_file.close();
  return true;
}

int DecodeJpgFrames(const string& curdir, const string& filename, const vector<char>& content) {
  const char* ptr = content.data();
  if (content.size() < 40 || !std::equal(ptr, ptr + 8, "IMOAVI\0\0") || !std::equal(ptr + 0x20, ptr + 0x28, "MOVIE\0\0\0")) {
    cout << "not a imoavi file.\n";
    return 1;
  }
  int save_idx{ 0 };
  int cnt = *(int*)(content.data() + 0x30);
  for (ptr = content.data() + 0x54; cnt-- && ptr < content.data() + content.size(); ) {
    uint32_t data_len = *(uint32_t*)ptr;
    ptr += 4;
    ptr += 4;
    if (data_len > 0) {
      SaveToFile(MakeFileName(curdir, filename, save_idx++, ".jpg"), { ptr, ptr + data_len });
    }
    ptr += data_len;

    ptr += 0x14; // skip padding
  }
  return 0;
}

int DecodeAbmp(const string& curdir, const string& filename, const vector<char>& content) {
  int save_idx{ 0 };
  const char* ptr = content.data();
  if (!std::equal(ptr, ptr + 16, "abmp12\0\0\0\0\0\0\0\0\0\0")) {
    cout << "not a abmp file.\n";
    return 1;
  }
  for (ptr = ptr + 16; ptr < content.data() + content.size(); ) {
    string type = ptr;
    if (type == "abdata12" || type == "abdata13" || type == "abdata14" || type == "abdata15") {
      ptr += 16;
      uint32_t data_len = *(uint32_t*)ptr;
      ptr += 4;
      SaveToFile(MakeFileName(curdir, filename, save_idx++, ".dat"), { ptr, ptr + data_len });
      ptr += data_len;
    }
    else if (type == "abimage10" || type == "absound10") {
      ptr += 16;
      uint8_t cnt = *(uint8_t*)ptr;
      ptr += 1;
      while (cnt--) {
        string type2 = ptr;
        if (type2 == "abimgdat11" || type2 == "abimgdat13" || type2 == "abimgdat14") {
          ptr += 16;
          uint16_t name_len = *(uint16_t*)ptr;
          ptr += 2;
          string name = JPToANSI({ ptr, ptr + name_len });
          ptr += name_len;
          uint16_t data_len = *(uint16_t*)ptr;
          ptr += 2;
          if (data_len > 0) {
            SaveToFile(MakeFileName(curdir, filename, save_idx++, "@" + name + ".dat"), { ptr, ptr + data_len });
          }
          ptr += data_len;

          uint8_t file_type = *(uint8_t*)ptr;
          ptr += 1;

          if (type2 == "abimgdat11")
            ptr += 0;
          else if (type2 == "abimgdat13")
            ptr += 12;
          else if (type2 == "abimgdat14")
            ptr += 76;
          else
            assert(0);

          switch (file_type) {
          case 1: {
            uint32_t data_len = *(uint32_t*)ptr;
            ptr += 4;
            SaveToFile(MakeFileName(curdir, filename, save_idx++, "@" + name + ".jpg"), { ptr, ptr + data_len });
            ptr += data_len;
            break;
          }
          case 2:
          case 3: {
            uint32_t data_len = *(uint32_t*)ptr;
            ptr += 4;
            if (data_len > 0) {
              SaveToFile(MakeFileName(curdir, filename, save_idx++, "@" + name + ".png"), { ptr, ptr + data_len });
            }
            ptr += data_len;
            break;
          }
          case 4: { // imoavi
            uint32_t data_len = *(uint32_t*)ptr;
            ptr += 4;
            if (data_len > 0) {
              DecodeJpgFrames(curdir, filename + "@" + name, { ptr, ptr + data_len });
            }
            ptr += data_len;
            break;
          }
          case 6: { // abmp
            uint32_t data_len = *(uint32_t*)ptr;
            ptr += 4;
            DecodeAbmp(curdir, filename + "@" + name, { ptr, ptr + data_len });
            ptr += data_len;
            break;
          }
          case 7: { // ogv
            uint32_t data_len = *(uint32_t*)ptr;
            ptr += 4;
            if (data_len > 0) {
              SaveToFile(MakeFileName(curdir, filename, save_idx++, "@" + name + ".ogg"), { ptr, ptr + data_len });
            }
            ptr += data_len;
            break;
          }
          case 8: { // model
            uint32_t data_len = *(uint32_t*)ptr;
            ptr += 4;
            if (data_len > 0) {
              SaveToFile(MakeFileName(curdir, filename, save_idx++, "@" + name), { ptr, ptr + data_len });
            }
            ptr += data_len;
            break;
          }
          default:
            assert(0);
            break;
          }
        } // "abimgdat13" or "abimgdat14"
        else if (type2 == "abimgdat15") {
          ptr += 16;
          uint32_t ver = *(uint32_t*)ptr;
          ptr += 4;
          uint16_t name_len = (*(uint16_t*)ptr) * 2;
          ptr += 2;
          string name = WCSToANSI(wstring((wchar_t*)ptr, (wchar_t*)(ptr + name_len)));
          ptr += name_len;
          uint16_t data_len = *(uint16_t*)ptr;
          ptr += 2;
          if (data_len > 0) {
            SaveToFile(MakeFileName(curdir, filename, save_idx++, "@" + name + ".dat"), { ptr, ptr + data_len });
          }
          ptr += data_len;

          uint8_t file_type = *(uint8_t*)ptr;
          ptr += 1;
          if (ver == 2)
            ptr += 29;
          else if (ver == 1)
            ptr += 17;
          else
            assert(0);

          switch (file_type) {
          case 1: {
            uint32_t data_len = *(uint32_t*)ptr;
            ptr += 4;
            SaveToFile(MakeFileName(curdir, filename, save_idx++, "@" + name + ".jpg"), { ptr, ptr + data_len });
            ptr += data_len;
            break;
          }
          case 2:
          case 3: {
            uint32_t data_len = *(uint32_t*)ptr;
            ptr += 4;
            if (data_len > 0) {
              SaveToFile(MakeFileName(curdir, filename, save_idx++, "@" + name + ".png"), { ptr, ptr + data_len });
            }
            ptr += data_len;
            break;
          }
          case 4: { // imoavi
            uint32_t data_len = *(uint32_t*)ptr;
            ptr += 4;
            if (data_len > 0) {
              DecodeJpgFrames(curdir + "\\" + filename, name, { ptr, ptr + data_len });
            }
            ptr += data_len;
            break;
          }
          case 6: { // abmp
            uint32_t data_len = *(uint32_t*)ptr;
            ptr += 4;
            DecodeAbmp(curdir, filename + "@" + name, { ptr, ptr + data_len });
            ptr += data_len;
            break;
          }
          case 7: { // ogv
            uint32_t data_len = *(uint32_t*)ptr;
            ptr += 4;
            if (data_len > 0) {
              SaveToFile(MakeFileName(curdir, filename, save_idx++, "@" + name + ".ogg"), { ptr, ptr + data_len });
            }
            ptr += data_len;
            break;
          }
          default:
            assert(0);
            break;
          }
        } // "abimgdat15"
        else if (type2 == "absnddat11") {
          ptr += 16;
          ptr += 9; // skip
        }
        else if (type2 == "absnddat12") {
          ptr += 16;
          uint32_t ver = *(uint32_t*)ptr;
          ptr += 4;
          uint16_t name_len = (*(uint16_t*)ptr) * 2;
          ptr += 2;
          string name = WCSToANSI(wstring((wchar_t*)ptr, (wchar_t*)(ptr + name_len)));
          ptr += name_len;
          uint16_t data_len = *(uint16_t*)ptr;
          ptr += 2;
          if (data_len > 0) {
            SaveToFile(MakeFileName(curdir, filename, save_idx++, "@" + name + ".dat"), { ptr, ptr + data_len });
          }
          ptr += data_len;
          if (ver == 1) {
            uint8_t type = *(uint8_t*)ptr;
            ptr += 1;
            if (type == 0) { // wav
              uint32_t data_len = *(uint32_t*)ptr;
              ptr += 4;
              if (data_len > 0) {
                SaveToFile(MakeFileName(curdir, filename, save_idx++, "@" + name + ".wav"), { ptr, ptr + data_len });
              }
              ptr += data_len;
            }
            else if (type == 1) { // ogg
              uint32_t data_len = *(uint32_t*)ptr;
              ptr += 4;
              if (data_len > 0) {
                SaveToFile(MakeFileName(curdir, filename, save_idx++, "@" + name + ".ogg"), { ptr, ptr + data_len });
              }
              ptr += data_len;
            }
            else
              assert(0);
          }
          else if (ver == 2) {
            ptr += 5; // skip
          }
          else
            assert(0);
        }
        else {
          assert(0);
        }
      } // while (cnt--)
    } // type == "abimage10" || type == "absound10"
    //else if (type == "abdata15")
    else {
      assert(0);
    }
  }
  return 0;
}

bool SplitPath(const std::string& full, std::string& drive, std::string& dir, std::string& file, std::string& ext)
{
  char _drive[MAX_PATH] = { 0 }, _dir[MAX_PATH] = { 0 }, _file[MAX_PATH] = { 0 }, _ext[MAX_PATH] = { 0 };
  errno_t err = _splitpath_s(full.c_str(), _drive, sizeof(_drive), _dir, sizeof(_dir), _file, sizeof(_file), _ext, sizeof(_ext));
  if (err != 0)
    return false;

  drive = _drive;
  dir = _dir;
  file = _file;
  ext = _ext;

  if (!dir.empty() && dir.back() == '\\') dir.pop_back();
  return true;
}

int main(int argc, char* argv[])
{
  if (argc != 2) return 0;

  string drv, cur_dir, file_name, ext;
  SplitPath(argv[1], drv, cur_dir, file_name, ext);
  ifstream in_file(argv[1], ios::binary);
  if (!in_file.is_open()) {
    cout << "invalid file.\n";
    return 1;
  }
  return DecodeAbmp(drv + cur_dir, file_name + ext, { istreambuf_iterator<char>(in_file), istreambuf_iterator<char>() });
}