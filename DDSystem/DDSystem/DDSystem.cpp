
#include <cassert>
#include <clocale>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "DDSystem.h"

using namespace std;


bool DDSystem::Open(const std::string & path)
{
	assert(mPackagePath.empty());

	mStream.open(path, ios::binary);
	if (!mStream.is_open())
	{
		return false;
	}

	mPackagePath = path;
	mStream.read((char*)&mHeader, sizeof(Header));
	return true;
}

void DDSystem::Close()
{
	mStream.close();
}

std::string DDSystem::ReadNullTerminateString(std::ifstream& stream)
{
	string str;
	char b;
	do
	{
		stream.read(&b, 1);
		str += b;
	} while (b != '\0');
    str.pop_back();     // drop '\0'
	return str;
}

std::string DDSystem::DetermineSuffix(std::vector<uint8_t>& plainData)
{
    if (!memcmp(plainData.data(), "OggS", 4))
        return "ogg";
    else if (!memcmp(plainData.data(), "DDSxHXB", 7))
        return "script";
    else if (!memcmp(plainData.data(), "RIFF", 4))
        return "wav";
    else if (!memcmp(plainData.data(), "BM", 2))
        return "bmp";
    else
    {
        if (plainData.size() > 0x12 &&
            (!memcmp(plainData.data(), "\x00\x00\x0a\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0xc) ||
             !memcmp(plainData.data(), "\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0xc)))
            return "tga";

        return "unknown";
    }
}

bool DDSystem::ExtractEntries()
{
	if (!mStream.is_open() || mPackagePath.empty())
		return false;

	if (mHeader.Magic == Header::MAGIC_2)
		return ExtractDDP2();
	else if (mHeader.Magic == Header::MAGIC_3)
		return ExtractDDP3();
	else
	{
		assert("unkonwn header magic" && 0);
		return false;
	}
}

bool DDSystem::ExtractDDP2()
{
	mStream.seekg(sizeof(Header), ios::beg);
	mEntries.clear();
	for (uint32_t i = 0; i < mHeader.EntryCount; ++i)
	{
		Entry2 e;
		mStream.read((char*)&e, sizeof(Entry2));

		NormalizedEntry entry;
		entry.Offset = e.Offset;
		entry.PlainSize = e.PlainSize;
		entry.PackedSize = e.PackedSize;

		ostringstream oss;
		oss << "DefaultName_" << setfill('0') << setw(3) << mDefaultName++;
		entry.NameWithoutSuffix = oss.str();

		mEntries.push_back(entry);
	}
	return true;
}

bool DDSystem::ExtractDDP3()
{
	mEntries.clear();

	for (uint32_t i = 0; i < mHeader.EntryCount; ++i)
	{
		Entry3_L1 L1;
		mStream.seekg(sizeof(Header) + i * sizeof(Entry3_L1), ios::beg);
		mStream.read((char*)&L1, sizeof(Entry3_L1));

        if (L1.BlockSize < sizeof(Entry3_L2_Header))
            continue;

		uint32_t readBytes = 0;
        mStream.seekg(L1.BlockOffset, ios::beg);
		while (readBytes < L1.BlockSize - sizeof(Entry3_L2_Header))
		{
			Entry3_L2_Header L2;
			mStream.read((char*)&L2, sizeof(Entry3_L2_Header));
			string str = ReadNullTerminateString(mStream);
			readBytes += L2.Size;
			assert(str.size() + sizeof(Entry3_L2_Header) <= L2.Size);

			NormalizedEntry entry;
			entry.Offset = L2.Offset;
			entry.PlainSize = L2.PlainSize;
			entry.PackedSize = L2.PackedSize == 0 ? L2.PlainSize : L2.PackedSize;
			entry.NameWithoutSuffix = str;
			assert(L2.PackedSize <= L2.PlainSize);
			mEntries.push_back(entry);
		}
	}
	return true;
}

bool DDSystem::ProcessScriptFile(std::vector<uint8_t>& plainData)
{
    if (plainData.size() < sizeof(ScriptCipherHeader))
    {
        cout << "script size too small.\n";
        return false;
    }

    ScriptCipherHeader* header = (ScriptCipherHeader*)plainData.data();
    uint32_t length = (header->BigEnddingSize[0] << 16) | (header->BigEnddingSize[1] << 8) | header->BigEnddingSize[2];
    if (length != plainData.size())
    {
        cout << "script size mismatch, want:" << hex << length << " actual:" << plainData.size() << endl;
        return false;
    }

    int key = (((length << 5) ^ 0xa5) * (length + 0x6f349)) ^ 0x34A9B129;
    uint32_t* p = (uint32_t*)(plainData.data() + sizeof(ScriptCipherHeader));
    uint32_t fourByteLength = (length - sizeof(ScriptCipherHeader)) / 4;
    for (uint32_t i = 0; i < fourByteLength; ++i)
        p[i] ^= key;
    uint8_t* pb = plainData.data() + sizeof(ScriptCipherHeader) + fourByteLength * 4;
    uint8_t* end = plainData.data() + plainData.size();
    for (int i = 0; pb + i < end; ++i)
        pb[i] ^= ((char*)&key)[i];

    if (header->Flag)
    {
        // 跳过两个字符串，供后续使用，对解码来说没啥用。。。
        //char* p = (char*)(plainData.data() + sizeof(ScriptCipherHeader));
        //while (*p++);
        //while (*p++);
    }
    return true;
}

bool DDSystem::ExtractResource(const std::string & saveDir)
{
	int saveCount = 0;

    // 创建目录
    string cmd("mkdir " + saveDir);
    system(cmd.c_str());

    std::setlocale(LC_ALL, "ja");
    std::wcout.imbue(std::locale("ja"));

	for (auto& entry : mEntries)
	{
		vector<uint8_t> packedData, plainData;
        packedData.resize(entry.PackedSize);
        plainData.resize(entry.PlainSize);
        mStream.seekg(entry.Offset, ios::beg);
		mStream.read((char*)packedData.data(), entry.PackedSize);
		if (entry.PackedSize != entry.PlainSize)
		{
			Decompress(plainData.data(), plainData.size(), packedData.data(), packedData.size());
		}
		else
		{
			plainData = packedData;
		}

        if (!memcmp(plainData.data(), "DDSxHXB", 7))
        {
            if (!ProcessScriptFile(plainData))
            {
                cout << "extract script [" + entry.NameWithoutSuffix + "] failed.\n";
                continue;
            }
        }

		string suffix = DetermineSuffix(plainData);
        string fileName = entry.NameWithoutSuffix;
        fileName += ".";
        fileName += suffix;
        string filePath = saveDir;
        filePath += fileName;

        vector<wchar_t> wstr;
        wstr.resize(512);
        mbstowcs_s(nullptr, wstr.data(), fileName.size(), fileName.c_str(), wstr.size());

		ofstream out;
		out.open(wstr.data(), ios::binary);
		if (!out.is_open())
		{
			cout << "create file [" + filePath + "] failed.\n";
			continue;
		}

		out.write((char*)plainData.data(), plainData.size());
		out.close();
		++saveCount;
        wcout << L"[" << wstr.data() << L"] saved.\n";
	}

	return saveCount == mEntries.size();
}


int DDSystem::Decompress(uint8_t* dst, int dstLen, uint8_t* src, int srcLen)
{
    uint8_t *v2; // ebp
    //unsigned int v3; // edx
    unsigned int v4; // eax
    int v9; // edi
    unsigned __int8 cur_cmd_byte; // cl
    unsigned __int8 *v12; // eax
    int v13; // ecx
    unsigned __int16 v14; // cx
    unsigned __int16 v15; // cx
    unsigned __int8 v16; // dl
    int v17; // ecx
    unsigned int v18; // edx
    char v19; // dl
    int v20; // ecx
    bool v21; // zf
    unsigned __int8 *v22; // eax
    int v23; // ecx
    unsigned __int16 v24; // cx
    unsigned __int16 v25; // cx
    int v26; // eax
    int v27; // eax
    unsigned int v28; // eax
    size_t v29; // edi
    int v30; // ecx
    uint8_t *i; // eax
    
    int decompressed_length = 0;
    int remain_length = srcLen;
    int flag0;  // v0x24
    int flag1 = 0;  // v0x20

    int loop_count = 0;

    v2 = dst;

    while (decompressed_length < srcLen)
    {
        if (!flag1)
        {
            v4 = remain_length;
            v9 = decompressed_length;
            cur_cmd_byte = *(uint8_t *)(src + v9);
            v12 = (unsigned __int8 *)(v9 + src + 1);
            if (cur_cmd_byte >= 0x20u)
            {
                if ((cur_cmd_byte & 0x80u) == 0)
                {
                    v19 = cur_cmd_byte & 0x60;
                    if ((cur_cmd_byte & 0x60) == 0x20)
                    {
                        v18 = ((unsigned int)cur_cmd_byte >> 2) & 7;
                        v17 = cur_cmd_byte & 3;
                    }
                    else
                    {
                        v20 = cur_cmd_byte & 0x1F;
                        v21 = v19 == 0x40;
                        v18 = *v12;
                        if (v21)
                        {
                            v17 = v20 + 4;
                            ++v12;
                        }
                        else
                        {
                            v22 = v12 + 1;
                            v18 |= v20 << 8;
                            v23 = *v22;
                            v12 = v22 + 1;
                            if (v23 == 0xfe)
                            {
                                *((uint8_t*)(&v24)+1) = *v12;
                                *(uint8_t*)(&v24) = v12[1];
                                v17 = v24 + 0x102;
                                v12 += 2;
                            }
                            else if (v23 == 0xff)
                            {
                                *((uint8_t*)(&v25)+1) = *v12;
                                *(uint8_t*)(&v25) = v12[1];
                                v17 = v12[3] | ((v12[2] | (v25 << 8)) << 8);
                                v12 += 4;
                            }
                            else
                            {
                                v17 = v23 + 4;
                            }
                        }
                    }
                }
                else
                {
                    v16 = cur_cmd_byte;
                    v17 = ((unsigned int)cur_cmd_byte >> 5) & 3;
                    v18 = *v12++ | ((v16 & 0x1F) << 8);
                }
                flag0 = v18 + 1;
                v13 = v17 + 3;
            }
            else if (cur_cmd_byte >= 0x1Du)
            {
                if (cur_cmd_byte == 29)
                {
                    v13 = *v12++ + 30;
                    flag0 = 0;
                }
                else if (cur_cmd_byte == 30)
                {
                    *((uint8_t*)(&v14)+1) = *v12;
                    *(uint8_t*)(&v14) = v12[1];
                    flag0 = 0;
                    v13 = v14 + 286;
                    v12 += 2;
                }
                else
                {
                    *((uint8_t*)(&v15)+1) = *v12;
                    *(uint8_t*)(&v15) = v12[1];
                    v13 = v12[3] | ((v12[2] | (v15 << 8)) << 8);
                    v12 += 4;
                    flag0 = 0;
                }
            }
            else
            {
                v13 = cur_cmd_byte + 1;
                flag0 = 0;
            }
            v26 = (int)(v12 - src);
            flag1 = v13;
            v27 = v26 - v9;
            remain_length -= v27;
            decompressed_length = v27 + v9;
        }
        v28 = flag1;
        v29 = flag1;
        v30 = flag0;
        flag1 = v28 - v29;
        if (v30)
        {
            for (i = &v2[-v30]; v29; ++i)
            {
                --v29;
                *v2++ = *i;
            }
        }
        else
        {
            if (remain_length < 0 || remain_length < v29)
            {
                assert(0);
            }
            memcpy(v2, (const void *)(decompressed_length + src), v29);
            decompressed_length += v29;
            v2 += v29;
        }
    }
    return 0;
}
