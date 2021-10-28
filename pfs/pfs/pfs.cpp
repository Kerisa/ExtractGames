
#include <cassert>
#include <iostream>
#include <fstream>

#include "pfs.h"
#include "openssl/sha.h"
#include <Windows.h>

using namespace std;

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

    HANDLE hFile;
    int ret = 0;
    do {
        hFile = CreateFileA(buf.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            ret = GetLastError();
            break;
        }

        BOOL success = WriteFile(hFile, unpackData.data(), unpackData.size(), &ByteWrite, NULL);

        if (success && ByteWrite == unpackData.size())
        {
            ret = ERROR_SUCCESS;
        }
        else
        {
            ret = GetLastError();
        }
    } while (0);

    CloseHandle(hFile);
    return ret;
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

std::string UTF8ToANSI(const std::string& str)
{
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, 0, 0);
    std::vector<wchar_t> wcs(size + 1);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wcs.data(), wcs.size());

    size = WideCharToMultiByte(CP_ACP, 0, wcs.data(), -1, 0, 0, 0, 0);
    std::vector<char> mbs(size + 1);
    WideCharToMultiByte(CP_ACP, 0, wcs.data(), -1, mbs.data(), mbs.size(), NULL, NULL);
    return mbs.data();
}

bool PfsFile::Open(const string& path) {
    if (!impl) {
        unique_ptr<PfsFileImpl> tmp2(new PfsFileImplV2());
        unique_ptr<PfsFileImpl> tmp6(new PfsFileImplV6());
        unique_ptr<PfsFileImpl> tmp8(new PfsFileImplV8());
        if (tmp2->Open(path)) {
            impl.swap(tmp2);
            return true;
        }
        else if (tmp6->Open(path)) {
            impl.swap(tmp6);
            return true;
        }
        else if (tmp8->Open(path)) {
            impl.swap(tmp8);
            return true;
        }
    }
    else {
        return impl->Open(path);
    }
    return false;
}

bool PfsFile::ExtractIndeies() {
    if (impl) {
        return impl->ExtractIndeies();
    }
    else {
        return false;
    }
}

bool PfsFile::ExtractResource()
{
    if (impl) {
        return impl->ExtractResource();
    }
    else {
        return false;
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "usage: " << argv[0] << " <pfs path>\n";
        return 1;
    }

    PfsFile pfs;
    if (!pfs.Open(argv[1])) {
        cout << "invalid pfs file\n";
        return 1;
    }

    if (!pfs.ExtractIndeies()) {
        cout << "extract index fail\n";
        return 1;
    }

    if (!pfs.ExtractResource()) {
        cout << "extract resource fail\n";
        return 1;
    }

    return 0;
}

bool PfsFileImplV2::Open(const string& path)
{
    ifstream infile(path, ios::binary);
    if (!infile.is_open())
        return false;
    vector<char> data(16);
    infile.seekg(0, ios::beg);
    infile.read(data.data(), data.size());
    auto magic = string(data.data(), 3);
    if (magic == "pf2") {
        mPath = path;
        mMagic = magic;
        mRelationFileDataOffset = *(uint32_t*)&data[3];
        mFileNumber = *(uint32_t*)&data[7];
        mFileStream.swap(infile);
        return true;
    }
    else {
        return false;
    }
}

bool PfsFileImplV2::ExtractIndeies()
{
    if (!mFileStream.is_open())
        return false;
    mFileStream.seekg(INDEX_START, ios::beg);
    vector<char> idx(mRelationFileDataOffset - INDEX_START);
    for (const char* p = idx.data(); p < idx.data() + idx.size(); ) {
        PfsEntry e;
        uint32_t len = *(uint32_t*)p;
        p += 4;
        e.mFileName.assign(p, len);
        p += len;
        assert(*(uint32_t*)p == 0x10);
        p += 4;
        assert(*(uint32_t*)p == 0);
        p += 4;
        assert(*(uint32_t*)p == 0);
        p += 4;
        e.mOffset = *(uint32_t*)p;
        p += 4;
        e.mSize = *(uint32_t*)p;
        p += 4;
        mEntries.push_back(e);
    }
    return mEntries.size() == mFileNumber;
}

bool PfsFileImplV2::ExtractResource()
{
    if (!mFileStream.is_open())
        return false;
    auto curDir = "[extract] " + mPath.substr(mPath.find_last_of('\\') + 1);
    for (auto e : mEntries) {
        mFileStream.seekg(e.mOffset, ios::beg);
        vector<char> data(e.mSize);
        mFileStream.read(data.data(), data.size());
        if (SplitFileNameAndSave(curDir, JPToANSI(e.mFileName), data) == 0) {
            cout << "[已保存] " << JPToANSI(e.mFileName) << "\n";
        }
        else {
            cout << "[无法保存] " << e.mFileName << "\n";
        }
    }
    return true;
}

bool PfsFileImplV6::Open(const string& path)
{
    ifstream infile(path, ios::binary);
    if (!infile.is_open())
        return false;
    vector<char> data(16);
    infile.seekg(0, ios::beg);
    infile.read(data.data(), data.size());
    auto magic = string(data.data(), 3);
    if (magic == "pf6") {
        mPath = path;
        mMagic = magic;
        mRelationFileDataOffset = *(uint32_t*)&data[3];
        mFileNumber = *(uint32_t*)&data[7];
        mFileStream.swap(infile);
        return true;
    }
    else {
        return false;
    }
}

bool PfsFileImplV6::ExtractIndeies()
{
    if (!mFileStream.is_open())
        return false;
    mFileStream.seekg(INDEX_START, ios::beg);
    vector<char> idx(mRelationFileDataOffset - INDEX_START);
    for (const char* p = idx.data(); p < idx.data() + idx.size(); ) {
        PfsEntry e;
        uint32_t len = *(uint32_t*)p;
        p += 4;
        e.mFileName.assign(p, len);
        p += len;
        p += 4;
        e.mOffset = *(uint32_t*)p;
        p += 4;
        e.mSize = *(uint32_t*)p;
        p += 4;
        mEntries.push_back(e);
    }
    return mEntries.size() == mFileNumber;
}

bool PfsFileImplV6::ExtractResource()
{
    if (!mFileStream.is_open())
        return false;
    auto curDir = "[extract] " + mPath.substr(mPath.find_last_of('\\') + 1);
    for (auto e : mEntries) {
        mFileStream.seekg(e.mOffset, ios::beg);
        vector<char> data(e.mSize);
        mFileStream.read(data.data(), data.size());
        if (SplitFileNameAndSave(curDir, JPToANSI(e.mFileName), data) == 0) {
            cout << "[已保存] " << JPToANSI(e.mFileName) << "\n";
        }
        else {
            cout << "[无法保存] " << e.mFileName << "\n";
        }
    }
    return true;
}

bool PfsFileImplV8::Open(const string& path)
{
    ifstream infile(path, ios::binary);
    if (!infile.is_open())
        return false;
    vector<char> data(16);
    infile.seekg(0, ios::beg);
    infile.read(data.data(), data.size());
    auto magic = string(data.data(), 3);
    if (magic == "pf8") {
        mPath = path;
        mMagic = magic;
        mRelationFileDataOffset = *(uint32_t*)&data[3];
        mFileNumber = *(uint32_t*)&data[7];
        mFileStream.swap(infile);
        return true;
    }
    else {
        return false;
    }
}

bool PfsFileImplV8::ExtractIndeies()
{
    if (!mFileStream.is_open())
        return false;

    static_assert(INDEX_HASH_START < INDEX_START, "");

    mFileStream.seekg(INDEX_HASH_START, ios::beg);
    vector<char> idx(mRelationFileDataOffset);
    mFileStream.read(idx.data(), idx.size());

    mXorKey.resize(20);
    SHA1((const uint8_t*)idx.data(), idx.size(), (uint8_t*)mXorKey.data());

    for (const char* p = idx.data() + (INDEX_START - INDEX_HASH_START); mEntries.size() < mFileNumber && p < idx.data() + idx.size(); ) {
        PfsEntry e;
        uint32_t len = *(uint32_t*)p;
        p += 4;
        e.mFileName.assign(p, len);
        p += len;
        p += 4;
        e.mOffset = *(uint32_t*)p;
        p += 4;
        e.mSize = *(uint32_t*)p;
        p += 4;
        mEntries.push_back(e);
    }
    return mEntries.size() == mFileNumber;
}

bool PfsFileImplV8::ExtractResource()
{
    if (!mFileStream.is_open())
        return false;

    auto curDir = "[extract] " + mPath.substr(mPath.find_last_of('\\') + 1);
    for (auto e : mEntries) {
        mFileStream.seekg(e.mOffset, ios::beg);
        vector<char> data(e.mSize);
        mFileStream.read(data.data(), data.size());
        auto plain = Xor(data);
        if (SplitFileNameAndSave(curDir, UTF8ToANSI(e.mFileName), plain) == 0) {
            cout << "[已保存] " << UTF8ToANSI(e.mFileName) << "\n";
        }
        else {
            cout << "[无法保存] " << e.mFileName << "\n";
        }
    }
    return true;
}

std::vector<char> PfsFileImplV8::Xor(const std::vector<char>& cipher)
{
    vector<char> copy(cipher);
    for (size_t i = 0; i < copy.size(); ++i) {
        copy[i] ^= mXorKey[i % mXorKey.size()];
    }
    return copy;
}
