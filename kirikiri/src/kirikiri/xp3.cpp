
#include "xp3.h"
#include <iterator>
#include <iostream>
#include <sstream>

using namespace std;


UNCOMPRESS unCom;



DWORD HaveExtraEntryChunk(const char *game)
{
    for (int i=_countof(XP3EntryExtraChunk)-1; i>=0; --i)
        if (!strcmp(game, XP3EntryExtraChunk[i].name))
        {
            return XP3EntryExtraChunk[i].chunk;
        }

    return 0;
}

int XP3ArcPraseEntryStage1 (
        PVOID _idx,
        DWORD _len,
        std::vector<file_entry>& Entry,
        DWORD chunk
        )
{
    /////////////////////////////////////////
    // Format:
    // <magic> <length>  <adlr>   <filename>
    // 4Bytes + 8Bytes + 4Bytes + 2Bytes+wcharstring
    /////////////////////////////////////////
    if (!chunk)
        return 1;

    PBYTE pEnd = (PBYTE)_idx + _len, p = (PBYTE)_idx;
    int walk = -1;
    while (p < pEnd && *(PDWORD)p != chunk) ++p;
    for (DWORD i=0; i<Entry.size(); ++i)
        if (*(PDWORD)(p+0xc) == Entry[i].checksum)
        {
            walk = i + 1;
            wchar_t temp[128] = { 0 };
            wcscpy_s(temp, _countof(temp), (wchar_t*)(p + 0x12));
            Entry[i].file_name = temp;
            p += 0x12 + wcslen((wchar_t*)(p + 0x12));
        }

    assert(walk != -1);

    while (p < pEnd)
    {
        if (*(PDWORD)p == chunk)
        {
            wchar_t temp[128] = { 0 };
            wcscpy_s(temp, _countof(temp), (wchar_t*)(p + 0x12));
            Entry[walk++].file_name = temp;
            p += 0x12 + wcslen((wchar_t*)(p + 0x12));
        }
        else
            ++p;
    }

    return 0;
}


UNCOMPRESS EncryptedXP3::unCom;

EncryptedXP3::EncryptedXP3()
{
    if (!unCom)
    {
        HMODULE hZlib = LoadLibrary(__T("zlib.dll"));
        assert(hZlib);
        unCom = (UNCOMPRESS)GetProcAddress(hZlib, "uncompress");
    }
}

EncryptedXP3::~EncryptedXP3()
{
    Close();
}

bool EncryptedXP3::Open(const std::wstring & path)
{
    assert(!mStream.is_open());
    mStream.open(path, ios::binary);
    if (!mStream.is_open())
    {
        assert("open failed xp3 packet" && 0);
        return false;
    }

    if (!IsValid())
    {
        Close();
        assert("not valid xp3 packet" && 0);
        return false;
    }

    mPath = path;
    return true;
}

void EncryptedXP3::Close()
{
    if (mStream.is_open())
        mStream.close();
    mPath.clear();
}

bool EncryptedXP3::IsValid()
{
    vector<char> magic(sizeof(mHeader.magic));
    assert(mStream.is_open());
    mStream.seekg(0, ios::beg);
    mStream.read(magic.data(), magic.size());

    return !memcmp(magic.data(), "XP3\r\n \n\x1A\x8B\x67\x01", magic.size());
}

std::vector<char> EncryptedXP3::GetPlainIndexBytes()
{
    mStream.seekg(0, ios::beg);
    mStream.read((char*)&mHeader.magic, sizeof(mHeader.magic));
    assert(!memcmp(mHeader.magic, "XP3\r\n \n\x1A\x8B\x67\x01", sizeof(mHeader.magic)));

    mStream.read((char*)&mHeader.offset, 8);

    if (mHeader.offset != 0x17)
    {
        mStream.seekg(mHeader.offset, ios::beg);
    }
    else
    {
        mStream.read((char*)&mHeader.minor_version, 4);
        mStream.read((char*)&mHeader.flag, 1);
        mStream.read((char*)&mHeader.index_size, 8);
        mStream.read((char*)&mHeader.index_offset, 8);
        mStream.seekg(mHeader.index_offset, ios::beg);
    }

    BYTE  idx_flag;
    uint64_t idx_size;
    uint64_t idx_uncom;

    mStream.read((char*)&idx_flag, 1);
    mStream.read((char*)&idx_size, 8);
    if (idx_flag)
    {
        mStream.read((char*)&idx_uncom, 8);
    }
    else
    {
        idx_uncom = idx_size;
    }

    vector<char> idx(idx_size);
    vector<char> idx_raw(idx_uncom);

    mStream.read(idx.data(), idx.size());
    if (idx_flag)
    {
        uint32_t detLen = (uint32_t)idx_uncom;
        unCom(idx_raw.data(), &detLen, idx.data(), idx_size);
    }
    else
    {
        idx_raw = idx;
    }

    return idx_raw;
}

std::vector<file_entry> EncryptedXP3::XP3ArcPraseEntryStage0(const std::vector<char>& plainBytes)
{
    std::vector<file_entry> Entry;


    static const DWORD _file = 0x656C6946, _adlr = 0x726c6461,
        _segm = 0x6d676573, _info = 0x6f666e69,
        flag_file = 0x1, flag_adlr = 0x2,
        flag_segm = 0x4, flag_info = 0x8,
        flag_all = 0xf;

    PBYTE p = (PBYTE)plainBytes.data();
    PBYTE pEnd = p + plainBytes.size();

    assert(*(PDWORD)p == _file);

    p += *(PDWORD)(p + 4) + 0xc;  // skip protection warning

    while (p < pEnd)
    {
        //////////////////////////////////////
        // 31<-------3----2----1--->0
        //          info segm adlr file
        //////////////////////////////////////
        int flag = 0;
        file_entry fe;
        memset(&fe, 0, sizeof(fe));
        PBYTE SingleEnd = pEnd;

        while (p < SingleEnd && flag != flag_all)
        {
            switch (*(PDWORD)p)
            {
            default:
                ++p;
                break;

            case _file:
                assert(!(flag & flag_file));
                SingleEnd = p + *(PDWORD)(p + 4) + 0xc;
                p += 0xc;
                flag |= flag_file;
                break;

            case _adlr:
                assert(!(flag & flag_adlr));
                p += 0xc;
                fe.checksum = *((PDWORD)p);
                p += 4;
                flag |= flag_adlr;
                break;

            case _segm:
                assert(!(flag & flag_segm));
                if (*(PDWORD)(p + 4) % 0x1c == 0)
                {
                    fe.part = *(PDWORD)(p + 4) / 0x1c;
                    p += 0xC;
                    for (int i = 0; i < fe.part; ++i)
                    {
                        fe.info[i].compress_flag = *(PDWORD)p;
                        p += 4;    // 1 compressed
                        fe.info[i].offset = *(PULONG64)p;
                        p += 8;
                        fe.info[i].orig_length = *(PULONG64)p;
                        p += 8;
                        fe.info[i].pkg_length = *(PULONG64)p;
                        p += 8;
                    }
                }
                else
                {
                    assert(L"错误的文件索引记录" && 0);
                    while (*(PDWORD)p != _file) ++p;    // 跳过这个索引
                }

                flag |= flag_segm;
                break;

            case _info: {
                assert(!(flag & flag_info));
                PBYTE info_sec_end = p + 0xc + *((PDWORD)(p + 0x4));
                p += 0xc;
                fe.encryption_flag = *((PDWORD)p);    // 好像这个标志也没啥用
                p += 0x14;  // 跳过info中的长度信息

                            // 剩下的是文件名长度和文件名
                int buf_size = (int)*((PWORD)p);
                p += 0x2;
                fe.file_name.assign((wchar_t*)p, (wchar_t*)p + buf_size);

                p = info_sec_end;

                flag |= flag_info;
                break;
            }
            }
        }   // end while (p < pEnd && flag != flag_all)

        assert(flag == flag_all);
        Entry.push_back(fe);
    }

    return Entry;
}

void EncryptedXP3::DumpEntriesToFile(const std::vector<file_entry>& entries, const std::wstring & path)
{
    ofstream out(path, ios::binary);
    if (!out.is_open())
    {
        wcout << L"DumpEntries: create file [" << path << L"] failed.\n";
        return;
    }

    for (size_t i = 0; i < entries.size(); ++i)
    {
        string str;
        str += "file name: ";

        int n = WideCharToMultiByte(CP_UTF8, NULL, entries[i].file_name.c_str(), -1, NULL, NULL, NULL, NULL);
        vector<char> mcb(n + 1);
        WideCharToMultiByte(CP_UTF8, NULL, entries[i].file_name.c_str(), -1, mcb.data(), n, NULL, NULL);
        str += mcb.data();

        stringstream s0;
        s0 << "\nchecksum: " << std::hex << entries[i].checksum << "\nencryption_flag: " << std::hex << entries[i].encryption_flag << "\n";
        str += s0.str();

        for (int k = 0; k < entries[i].part; ++k)
        {
            stringstream ss;
            ss << "parts " << k + 1 << ":\n    ";
            ss << "offset: " << std::hex << entries[i].info[k].offset << "\n    ";
            ss << "orignal: " << std::hex << entries[i].info[k].orig_length << "\n    ";
            ss << "packet: " << std::hex << entries[i].info[k].pkg_length << "\n    ";
            ss << "compress_flag: " << std::hex << entries[i].info[k].compress_flag << "\n";
            str += ss.str();
        }

        str += "---------------------------------------------\n\n";
        out.write(str.c_str(), str.size());
    }

    out.close();
}

std::vector<file_entry> EncryptedXP3::ExtractEntries(const std::vector<char>& plainBytes)
{
    return XP3ArcPraseEntryStage0(plainBytes);
}

bool EncryptedXP3::ReadEntryDataOfAllParts(const file_entry & fe, vector<char>& packedData, uint32_t* pOriginalLength)
{
    fe.ReadFileData(mStream, packedData);
    *pOriginalLength = fe.GetTotleOriginalSize();
    return true;
}

int EncryptedXP3::ExtractData(const std::vector<file_entry>& Entry, const std::wstring& saveDir, wostream& output)
{
    uint32_t cnt_savefile = 0;

    for (const file_entry& fe : Entry)
    {
        vector<char> cipher;
        uint32_t file_org_len = 0;
        ReadEntryDataOfAllParts(fe, cipher, &file_org_len);

        vector<char> unpack(file_org_len);
        if (fe.info[0].compress_flag)
        {
            uint32_t unpack_len = (uint32_t)file_org_len;
            unCom(unpack.data(), &unpack_len, cipher.data(), cipher.size());
        }
        else
        {
            assert(cipher.size() == file_org_len);
            unpack = cipher;
        }


        if (DoExtractData(fe, unpack))
        {
            if (!SplitFileNameAndSave(saveDir.c_str(), fe.file_name, unpack))
            {
                ++cnt_savefile;
                output << wstring(wstring(L"[已保存]") + fe.file_name + L"\r\n");
            }
            else
            {
                output << wstring(wstring(L"[无法保存]") + fe.file_name + L"\r\n");
            }
        }
    }

    return cnt_savefile;
}

bool EncryptedXP3::DoExtractData(const file_entry& fe, std::vector<char>& unpackData)
{
    // 默认
    return true;
}

int EncryptedXP3::SplitFileNameAndSave(const wstring& cur_dir, const wstring& file_name, const vector<char>& unpackData)
{
    DWORD ByteWrite;
    wstring buf;

    buf = cur_dir + L"\\" + file_name;

    int len = buf.size();
    int i = cur_dir.size() + 1;
    const wchar_t* p = buf.c_str();
    const wchar_t* end = buf.c_str() + len;
    while (p <= end && i < len)
    {
        while (buf[i] != '\\' && buf[i] != '/' && buf[i] != '\0') ++i;
        if (buf[i] == '/') buf[i] = '\\';
        if (i<len)
        {
            wchar_t tmp = buf[i];
            buf[i] = '\0';

            CreateDirectoryW(p, 0);
            buf[i] = tmp;
            ++i;
        }
    }

    wstring buf2;
    HANDLE hFile;
    int ret = 0;
    do {
        hFile = CreateFileW(buf.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
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





std::vector<file_entry> palette_9_nine::XP3ArcPraseEntryStage0(const std::vector<char>& plainBytes)
{
    std::vector<file_entry> Entry;
    std::vector<file_entry> nameList;


    static const DWORD _file = 0x656C6946, _adlr = 0x726c6461,
        _segm = 0x6d676573, _info = 0x6f666e69, _hnfn = 0x6e666e68,
        flag_file = 0x1, flag_adlr = 0x2,
        flag_segm = 0x4, flag_info = 0x8,
        flag_all = 0xf;

    PBYTE p = (PBYTE)plainBytes.data();
    PBYTE pEnd = p + plainBytes.size();

    assert(*(PDWORD)p == _hnfn);
    p += *(PDWORD)(p + 4) + 0xc;  // skip protection warning

    while (*(PDWORD)p == _hnfn)
    {
        uint64_t length = *(uint64_t*)(p + 4);
        uint32_t checksum = *(uint32_t*)(p + 12);
        uint32_t nameLen = *(uint16_t*)(p + 16);
        std::wstring name = (wchar_t*)(p + 18);
        p += length + 12;

        file_entry fe;
        fe.checksum = checksum;
        fe.file_name = name;
        nameList.push_back(fe);
    }

    while (p < pEnd)
    {
        //////////////////////////////////////
        // 31<-------3----2----1--->0
        //          info segm adlr file
        //////////////////////////////////////
        int flag = 0;
        file_entry fe;
        memset(&fe, 0, sizeof(fe));
        PBYTE SingleEnd = pEnd;

        while (p < SingleEnd && flag != flag_all)
        {
            switch (*(PDWORD)p)
            {
            default:
                ++p;
                break;

            case _file:
                assert(!(flag & flag_file));
                SingleEnd = p + *(PDWORD)(p + 4) + 0xc;
                p += 0xc;
                flag |= flag_file;
                break;

            case _adlr:
                assert(!(flag & flag_adlr));
                p += 0xc;
                fe.checksum = *((PDWORD)p);
                p += 4;
                flag |= flag_adlr;
                break;

            case _segm:
                assert(!(flag & flag_segm));
                if (*(PDWORD)(p + 4) % 0x1c == 0)
                {
                    fe.part = *(PDWORD)(p + 4) / 0x1c;
                    p += 0xC;
                    for (int i = 0; i < fe.part; ++i)
                    {
                        fe.info[i].compress_flag = *(PDWORD)p;
                        p += 4;    // 1 compressed
                        fe.info[i].offset = *(PULONG64)p;
                        p += 8;
                        fe.info[i].orig_length = *(PULONG64)p;
                        p += 8;
                        fe.info[i].pkg_length = *(PULONG64)p;
                        p += 8;
                    }
                }
                else
                {
                    assert(L"错误的文件索引记录" && 0);
                    while (*(PDWORD)p != _file) ++p;    // 跳过这个索引
                }

                flag |= flag_segm;
                break;

            case _info: {
                assert(!(flag & flag_info));
                PBYTE info_sec_end = p + 0xc + *((PDWORD)(p + 0x4));
                p += 0xc;
                fe.encryption_flag = *((PDWORD)p);    // 好像这个标志也没啥用
                p += 0x14;  // 跳过info中的长度信息

                            // 剩下的是文件名长度和文件名
                int buf_size = (int)*((PWORD)p);
                p += 0x2;
                fe.file_name.assign((wchar_t*)p, (wchar_t*)p + buf_size);

                p = info_sec_end;

                flag |= flag_info;
                break;
            }
            }
        }   // end while (p < pEnd && flag != flag_all)

        assert(flag == flag_all);
        Entry.push_back(fe);
    }

    assert(Entry.size() >= nameList.size());
    size_t n = 0;
    for (size_t i = 0; i < Entry.size(); ++i)
    {
        if (Entry[i].checksum != nameList[n].checksum)
            continue;
        Entry[i].file_name = nameList[n].file_name;
        ++n;
    }

    assert(n == nameList.size());
    return Entry;
}

uint32_t file_entry::GetTotlePackedSize() const
{
    uint32_t file_pkg_len = 0;
    for (int i = 0; i < part; ++i)
    {
        file_pkg_len += (uint32_t)info[i].pkg_length;
    }
    return file_pkg_len;
}

uint32_t file_entry::GetTotleOriginalSize() const
{
    uint32_t file_org_len = 0;
    for (int i = 0; i < part; ++i)
    {
        file_org_len += (uint32_t)info[i].orig_length;
    }
    return file_org_len;
}

bool file_entry::IsCompressed() const
{
    assert(part > 0);
    return info[0].compress_flag;
}

bool file_entry::IsEncrypted() const
{
    return encryption_flag;
}

bool file_entry::ReadFileData(std::ifstream & file, std::vector<char>& packedData) const
{
    uint32_t file_pkg_len = GetTotlePackedSize();
    uint32_t file_read = 0;
    packedData.resize(file_pkg_len);

    for (int i = 0; i < part; ++i)
    {
        file.seekg(info[i].offset, ios::beg);
        file.read(packedData.data() + file_read, (uint32_t)info[i].pkg_length);
        file_read += (uint32_t)info[i].pkg_length;
    }

    return true;
}