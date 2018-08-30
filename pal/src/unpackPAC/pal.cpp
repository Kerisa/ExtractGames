
#include "pal.h"
#include <cassert>
#include <iostream>

using namespace std;


bool PAL::Open(const std::string & path)
{
    assert(mPath.empty());

    mStream.open(path, ios::binary);
    if (!mStream.is_open())
    {
        cout << path << " open failed.\n";
        return false;
    }

    mStream.read((char*)&mHeader, sizeof(PACHeader));

    if (mHeader.Magic != mHeader.MAGIC)
    {
        cout << "not a valid pac package.\n";
        Close();
        return false;
    }

    mPath = path;
    return true;
}

void PAL::Close()
{
    mPath = "";
    mStream.close();
}

bool PAL::ExtractEntries()
{
    assert(!mPath.empty() && mStream.is_open());
    mEntries.resize(mHeader.mEntryCount);
    mStream.seekg(sizeof(PACHeader), ios::beg);
    for (uint32_t i = 0; i < mHeader.mEntryCount; ++i)
    {
        mStream.read((char*)&mEntries[i], sizeof(PACEntry));
    }

    char buffer[16];
    for (uint32_t i = 0; i < mHeader.mEntryCount; ++i)
    {
        memset(buffer, 0, sizeof(buffer));
        mStream.seekg(mEntries[i].e.Offset, ios::beg);
        mStream.read(buffer, 4);
        mEntries[i].Extra = buffer;
        mEntries[i].Type = NormalizedEntry::Base;
        if (mEntries[i].Extra == "PGD3")
            mEntries[i].Type = NormalizedEntry::Difference;
        else if (mEntries[i].Extra == "PGD2")
            mEntries[i].Type = NormalizedEntry::Difference;
    }
    return true;
}

bool PAL::ExtractResource(const std::string & saveDir)
{
    string cmd("mkdir \"");
    cmd += saveDir.c_str();
    cmd += "\"";
    system(cmd.c_str());

    int saveCount = 0;
    vector<uint8_t> plainData;
    for (auto& e : mEntries)
    {
        mStream.seekg(e.e.Offset, ios::beg);
        if (plainData.size() < e.e.PackLength)
            plainData.resize(e.e.PackLength * 2);
        mStream.read((char*)plainData.data(), e.e.PackLength);

        if (plainData[0] == '$' && string(e.e.Filename).find(".DAT") != string::npos)
            DecodeScript(plainData, e.e.PackLength);

        ofstream out(saveDir + e.e.Filename, ios::binary);
        if (out)
        {
            out.write((const char*)plainData.data(), e.e.PackLength);
            out.close();
            ++saveCount;
            cout << "[" << e.e.Filename << "] saved.\n";
        }
        else
        {
            cout << "create file [" << e.e.Filename << "] failed.\n";
        }
    }

    return saveCount == mEntries.size();
}

void PAL::DecodeScript(vector<uint8_t>& data, size_t dataLength)
{
    if (data.size() < 16)
        return;

    const uint32_t key1 = 0x84df873;
    const uint32_t key2 = 0xff987dee;

    uint8_t shift = 4;
    size_t oldSize = dataLength & ~3;

    uint8_t* ptr = data.data() + 16;
    uint8_t* pend = data.data() + oldSize;
    while (ptr < pend)
    {
        uint8_t mod = shift % 8;
        *ptr = (*ptr >> (8 - mod)) | (*ptr << mod);
        *(uint32_t*)ptr ^= key1;
        *(uint32_t*)ptr ^= key2;
        ++shift;
        ptr += 4;
    }
}
