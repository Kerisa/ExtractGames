
#include "leaf.h"
#include <cassert>
#include <windows.h>

using namespace std;

bool Leaf::Open(const std::string & file)
{
    assert(!mPackage.is_open());
    mPackage.open(file, ios::binary);
        
    if (!mPackage.is_open())
        return false;

    char buf[32] = { 0 };
    mPackage.read(buf, sizeof(buf));    
    switch (*(uint32_t*)buf)
    {
    case Header::kLacMagic:  // "LAC"
        mHeader.magic        = *(uint32_t*)buf;
        mHeader.entry_number = *(uint32_t*)(buf + 4);
        break;

    case Header::kKcapMagic:    // "KCAP"
        static_assert(Header::kKcapSize <= sizeof(buf), "Header::kKcapSize <= sizeof(buf)");
        memcpy_s(&mHeader, Header::kKcapSize, buf, Header::kKcapSize);
        break;

    default:
        assert(0);
        mPackage.close();
        return false;
    }

    mPath = file;
    return true;
}

bool Leaf::ExtractEntries()
{
    assert(mPackage.is_open());
    if (mHeader.magic == Header::kLacMagic)
    {
        mPackage.seekg(Header::kLacSize, ios::beg);
        for (uint32_t i = 0; i < mHeader.entry_number; ++i)
        {
            Entry e;
            char name[0x18] = { 0 };
            mPackage.read(name, sizeof(name));
            e.name = name;
            for (size_t i = 0; i < e.name.size(); ++i) e.name[i] ^= 0xff;
            e.name = JPtoGBK(e.name);
            mPackage.read((char*)&e.unknown2, 4);
            mPackage.read((char*)&e.unknown3, 4);
            mPackage.read((char*)&e.pack_size, 4);
            mPackage.read((char*)&e.offset, 4);
            mEntries.push_back(e);
        }
    }
    else if (mHeader.magic == Header::kKcapMagic)
    {
        mPackage.seekg(Header::kKcapSize, ios::beg);
        for (uint32_t i = 0; i < mHeader.entry_number; ++i)
        {
            Entry e;
            char name[0x18] = { 0 };
            mPackage.read((char*)&e.is_compressed, 4);
            mPackage.read(name, sizeof(name));
            e.name = JPtoGBK(name);
            mPackage.read((char*)&e.unknown2, 4);
            mPackage.read((char*)&e.unknown3, 4);
            mPackage.read((char*)&e.offset, 4);
            mPackage.read((char*)&e.pack_size, 4);
            mEntries.push_back(e);
        }
    }
    else
    {
        return false;
    }

    return true;
}

bool Leaf::ExtractResource()
{
    assert(mPackage.is_open());
    if (mHeader.magic == Header::kLacMagic)
    {
        for (uint32_t i = 0; i < mEntries.size(); ++i)
        {
            const Entry& e = mEntries[i];
            mPackage.seekg(e.offset, ios::beg);
            vector<char> data(e.pack_size);
            mPackage.read(data.data(), data.size());
            ofstream out(e.name.c_str(), ios::binary);
            out.write(data.data(), data.size());
            out.close();
        }
    }
    else if (mHeader.magic == Header::kKcapMagic)
    {
        for (uint32_t i = 0; i < mEntries.size(); ++i)
        {
            const Entry& e = mEntries[i];
            if (e.pack_size == 0)
                continue;

            mPackage.seekg(e.offset, ios::beg);
            vector<char> data(e.pack_size);
            vector<char> plain;
            mPackage.read(data.data(), data.size());
            if (e.is_compressed)
            {
                assert(*(uint32_t*)data.data() == data.size() || *(uint32_t*)data.data() + 8 == data.size());
                plain.resize(*(uint32_t*)(data.data() + 4));
                int size = Decompress(plain.data(), data.data() + 8, data.size() - 8, plain.size());
                assert(size == plain.size());
            }
            ofstream out(e.name.c_str(), ios::binary);
            out.write(plain.data(), plain.size());
            out.close();
        }
    }
    else
    {
        return false;
    }

    return true;
}


std::string Leaf::JPtoGBK(const std::string & str)
{

    int size = MultiByteToWideChar(932, 0, str.c_str(), -1, 0, 0);
    vector<wchar_t> wcs(size + 1);
    MultiByteToWideChar(932, 0, str.c_str(), -1, wcs.data(), wcs.size());

    size = WideCharToMultiByte(CP_ACP, 0, wcs.data(), -1, 0, 0, 0, 0);
    vector<char> mbs(size + 1);
    WideCharToMultiByte(CP_ACP, 0, wcs.data(), -1, mbs.data(), mbs.size(), NULL, NULL);
    return mbs.data();
}

int Leaf::Decompress(char* output, char* input, int input_size, int output_size)
{
    int v4; // ebp
    char *v5; // edi
    unsigned __int8 *v6; // ebx
    signed int v7; // esi
    int v8; // edx
    int v9; // eax
    int v10; // edx
    unsigned __int8 v11; // cl
    int v12; // eax
    int v13; // edx
    char v14; // cl
    char v16; // [esp+0h] [ebp-1Ch]
    unsigned int i; // [esp+10h] [ebp-Ch]
    int v18; // [esp+14h] [ebp-8h]
    signed int v19; // [esp+18h] [ebp-4h]
    unsigned __int8 *v20; // [esp+20h] [ebp+4h]

    vector<uint8_t> alloc(0x1000);
    uint8_t* byte_77CE50 = alloc.data();

    v4 = 0;
    v18 = 0;
    v19 = 0;
    memset(byte_77CE50, 32, 0xFEEu);
    v5 = output;
    v6 = (unsigned char*)input;
    v7 = 4078;
    for (i = 0; ; i >>= 1)
    {
        v8 = v18;
        if (!(i & 0x100))
        {
            if (v18 >= input_size)
            {
                assert("‚P", v16);
                goto LABEL_19;
            }
            v8 = v18 + 1;
            i = *v6++ | 0xFF00;
        }
        if (v8 >= input_size)
        {
            assert("‚Q", v16);
            goto LABEL_19;
        }
        v9 = *v6;
        v10 = v8 + 1;
        ++v6;
        v18 = v10;
        if (i & 1)
        {
            byte_77CE50[v7 & 0xFFF] = v9;
            *v5++ = v9;
            ++v7;
            ++v4;
            goto LABEL_14;
        }
        if (v10 >= input_size)
            break;
        v11 = *v6;
        v18 = v10 + 1;
        ++v6;
        v12 = 16 * (v11 & 0xF0) | v9;
        v20 = v6;
        v13 = (v11 & 0xF) + 2;
        if (v13 >= 0)
        {
            while (v4 <= output_size)
            {
                v14 = byte_77CE50[v12 & 0xFFF];
                byte_77CE50[v7 & 0xFFF] = v14;
                v6 = v20;
                *v5 = v14;
                --v13;
                ++v12;
                ++v7;
                ++v5;
                ++v4;
                if (v13 < 0)
                    goto LABEL_14;
            }
            v19 = 1;
        }
    LABEL_14:
        if (v4 > output_size)
            goto LABEL_20;
    }
    assert("‚R", v16);
LABEL_19:
    if (!v19)
        return v7 - 4078;
LABEL_20:
    assert(0, "³öÏÖ´íÎó¡¡" && 0);
    return v7 - 4078;
}
