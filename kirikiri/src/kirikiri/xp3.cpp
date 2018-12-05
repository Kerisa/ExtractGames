
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
            wcscpy_s(Entry[i].file_name, file_entry::StrCapacity, (wchar_t*)(p + 0x12));
            p += 0x12 + wcslen((wchar_t*)(p + 0x12));
        }

    assert(walk != -1);

    while (p < pEnd)
    {
        if (*(PDWORD)p == chunk)
        {
            wcscpy_s(Entry[walk++].file_name, file_entry::StrCapacity, (wchar_t*)(p + 0x12));
            p += 0x12 + wcslen((wchar_t*)(p + 0x12));
        }
        else
            ++p;
    }

    return 0;
}


void XP3Entrance(const wchar_t *packName, const wchar_t *curDirectory, const std::wstring& choosedGame)
{
    DWORD idx_size = 0;
    wchar_t szBuffer[MAX_PATH];

    auto fucker = CreateXP3Handler(choosedGame);
    if (!fucker)
    {
        StringCchPrintf(szBuffer, MAX_PATH, L"无法处理的游戏[%s]\r\n", choosedGame.c_str());
        AppendMsg(szBuffer);
        return;
    }

    bool success = fucker->Open(packName);
    assert(success);
    if (!success)
    {
        delete fucker;
        return;
    }

    wstring txt(packName);
    txt += L"_entries.txt";
    auto rawBytes = fucker->GetPlainIndexBytes();
    ofstream idx(txt + L".0.txt");
    assert(idx.is_open());
    idx.write(rawBytes.data(), rawBytes.size());
    idx.close();

    auto entries = fucker->ExtractEntries(rawBytes);
    fucker->DumpEntriesToFile(entries, txt.c_str());
    int saveFileCount = fucker->ExtractData(entries, curDirectory);
    fucker->Close();
    delete fucker;

    if (entries.size() == saveFileCount)
    {
        StringCchPrintf(szBuffer, MAX_PATH,
            L"[提取完成(%d/%d)]%s\r\n", saveFileCount, saveFileCount, packName);
        AppendMsg(szBuffer);
    }
    else
    {
        StringCchPrintf(szBuffer, MAX_PATH, L"提取%d个文件，共%d个，有%d个发生错误\r\n%s\r\n",
            saveFileCount, entries.size(), entries.size() - saveFileCount, packName);
        MessageBox(0, szBuffer, L"提示", MB_ICONWARNING);
    }
}

std::map<std::wstring, GameInfomation> GameNameMap = {
    { L"<默认：直接提取>", { "Unencrypted", []() { return new EncryptedXP3; }} },
    { L"さくら、咲きました。", { "sakurasaki", []() { return nullptr; } } },
    { L"倉野くんちのふたご事情", { "kuranokunchi", []() { return new kuranokunchi; }} },
    { L"あま恋シロップス ～恥じらう恋心でシたくなる甘神様の恋祭り～", { "amakoi", []() { return new amakoi; } } },
    { L"カミツレ ～7の二乗不思議～", { "Unencrypted", []() { return new EncryptedXP3; } } },
    { L"すたーらいと★アイドル -COLORFUL TOP STAGE！-", { "Unencrypted", []() { return new EncryptedXP3; } } },
    { L"あまたらすリドルスター",{ "Unencrypted", []() { return new EncryptedXP3; } } },
    { L"カラフル☆きゅあ～ 缤纷少女", { "colorfulcure", []() { return new colorfulcure; }} },
    { L"ＰＲＥＴＴＹ×Ｃ∧ＴＩＯＮ", { "prettycation", []() { return new prettycation; }} },
    { L"your diary ＋H",{ "kuranokunchi", []() { return new kuranokunchi; } } },
    { L"迷える2人とセカイのすべて",{ "Unencrypted", []() { return new EncryptedXP3; } } },
    { L"迷える2人とセカイのすべて LOVEHEAVEN300％",{ "Unencrypted", []() { return new EncryptedXP3; } } },
    { L"ヤリまん娘 ～俺の妹はビチビチビッチ～",{ "Unencrypted", []() { return new EncryptedXP3; } } },
    { L"公園いたずらシミュレータ ver.MAKO",{ "Unencrypted", []() { return new EncryptedXP3; } } },
    { L"PRETTY×CATION2", { "prettycation", []() { return new prettycation; } } },
    { L"エロ本を捨ててから兄の様子がおかしい", { "anioka", []() { return nullptr; } } },
    { L"SWANSONG", { "swansong", []() { return new swansong; }} },
    { L"恋がさくころ桜どき", { "koisakura", []() { return nullptr; } } },
    { L"ずっとすきして たくさんすきして", { "sukisuki", []() { return nullptr; } } },
    { L"俺と5人の嫁さんがラブラブなのは、未来からきた赤ちゃんのおかげに違いない！？", { "oreaka", []() { return nullptr; } } },
    { L"妹のセイイキ(未完成)", { "seiiki", []() { return nullptr; } } },
    { L"オトメ＊ドメイン", { "Otomedomain", []() { return nullptr; } } },
    { L"LOVELY×CATION2", { "lovelycation", []() { return new lovelycation; }} },
    { L"出会って5分は俺のもの！ 時間停止と不可避な運命", { "deai5bu", []() { return new deai5bu; }} },
    { L"神頼みしすぎて俺の未来がヤバい。", { "kamiyabai", []() { return new kamiyabai; }} },
    { L"9-nine-ここのつここのかここのいろ", { "palette 9-nine", []() { return new palette_9_nine; } } },
};

EncryptedXP3 * CreateXP3Handler(const std::wstring & gameName)
{
    //static map<string, std::function<EncryptedXP3*()>> list{
    //    { "kuranokunchi",   []() { return new kuranokunchi; } },
    //    { "amakoi",         []() { return new amakoi;       } },
    //    { "prettycation",   []() { return new prettycation; } },
    //    { "lovelycation",   []() { return new lovelycation; } },
    //    { "swansong",       []() { return new swansong;     } },
    //    { "deai5bu",        []() { return new deai5bu;      } },
    //    { "kamiyabai",      []() { return new kamiyabai;    } },

    //    // cxdec
    //    { "colorfulcure",   []() { return new colorfulcure; } },
    //};

    //auto it = list.find(gameName);
    //if (it != list.end())
    //    return it->second();
    //else
    //    return new EncryptedXP3;

    auto it = GameNameMap.find(gameName);
    assert(it != GameNameMap.end());
    return it->second.Handler();
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
                {   // 不应该进来
                    assert(0);
                    AppendMsg(L"错误的文件索引记录\r\n");
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
                if (buf_size >= _countof(fe.file_name))
                {
                    MessageBox(0, L"文件名超出缓冲区长度\r\n", L"提示", MB_ICONWARNING | MB_OK);
                    buf_size = _countof(fe.file_name) - 1;
                }
                p += 0x2;
                memset(fe.file_name, 0, _countof(fe.file_name));
                memcpy(fe.file_name, (wchar_t*)p, buf_size * sizeof(wchar_t));

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

        int n = WideCharToMultiByte(CP_UTF8, NULL, entries[i].file_name, -1, NULL, NULL, NULL, NULL);
        vector<char> mcb(n + 1);
        WideCharToMultiByte(CP_UTF8, NULL, entries[i].file_name, -1, mcb.data(), n, NULL, NULL);
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

int EncryptedXP3::ExtractData(const std::vector<file_entry>& Entry, const std::wstring& saveDir)
{
    uint32_t cnt_savefile = 0;

    for (const file_entry& fe : Entry)
    {
        uint32_t file_pkg_len = 0;
        uint32_t file_org_len = 0;
        for (int i = 0; i<fe.part; ++i)
        {
            file_pkg_len += (uint32_t)fe.info[i].pkg_length;
            file_org_len += (uint32_t)fe.info[i].orig_length;
        }

        uint32_t file_read = 0;
        vector<char> cipher(file_pkg_len);

        for (int i = 0; i<fe.part; ++i)
        {
            mStream.seekg(fe.info[i].offset, ios::beg);
            mStream.read(cipher.data() + file_read, (uint32_t)fe.info[i].pkg_length);
            file_read += (uint32_t)fe.info[i].pkg_length;
        }

        vector<char> unpack(file_org_len);
        if (fe.info[0].compress_flag)
        {
            uint32_t unpack_len = (uint32_t)file_org_len;
            unCom(unpack.data(), &unpack_len, cipher.data(), file_pkg_len);
        }
        else
        {
            assert(file_pkg_len == file_org_len);
            unpack = cipher;
        }


        if (DoExtractData(fe, unpack))
            if (!SplitFileNameAndSave(saveDir.c_str(), fe.file_name, unpack))
                ++cnt_savefile;

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
        hFile = CreateFile(buf.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            buf2 = L"[文件创建错误]" + file_name + L"\r\n";
            ret = ERR_FILE_CREATE;
            break;
        }

        WriteFile(hFile, unpackData.data(), unpackData.size(), &ByteWrite, NULL);

        if (ByteWrite != unpackData.size())
        {
            buf2 = L"[文件写入错误]" + file_name + L"\r\n";
            ret = ERR_FILE_ERITE;
            break;
        }

        int t = GetLastError();
        if (!t || t == ERROR_ALREADY_EXISTS)
        {
            buf2 = L"[已保存]" + file_name + L"\r\n";
        }
        else
        {
            buf2 = L"[无法保存]" + file_name + L"\r\n";
            ret = ERR_FILE_OTHERS;
        }
    } while (0);

    AppendMsg(buf2.c_str());
    CloseHandle(hFile);
    return ret;
}




bool kuranokunchi::DoExtractData(const file_entry & fe, std::vector<char>& unpackData)
{
    for (size_t i = 0; i<unpackData.size(); ++i)
        unpackData[i] ^= (uint8_t)fe.checksum ^ 0xcd;
    return true;
}

bool amakoi::DoExtractData(const file_entry & fe, std::vector<char>& unpackData)
{
    for (size_t i = 0; i<unpackData.size(); ++i)
        unpackData[i] ^= (uint8_t)fe.checksum;
    return true;
}

bool prettycation::DoExtractData(const file_entry & fe, std::vector<char>& unpackData)
{
    for (size_t i = 5; i<unpackData.size(); ++i)
        unpackData[i] ^= (uint8_t)(fe.checksum >> 0xc);
    return true;
}

bool lovelycation::DoExtractData(const file_entry & fe, std::vector<char>& unpackData)
{
    char* buf = unpackData.data();

    uint8_t key[5];
    key[0] = (uint8_t)(fe.checksum >> 8) & 0xff;
    key[1] = (uint8_t)(fe.checksum >> 8) & 0xff;
    key[2] = (uint8_t)(fe.checksum >> 1) & 0xff;
    key[3] = (uint8_t)(fe.checksum >> 7) & 0xff;
    key[4] = (uint8_t)(fe.checksum >> 5) & 0xff;

    for (size_t i = 0; i <= 0x64; ++i)
    {
        *buf++ ^= key[4];
    }
    for (size_t i = 0x65; i<unpackData.size(); ++i)
    {
        *buf++ ^= key[i & 4];
    }
    return true;
}

bool swansong::DoExtractData(const file_entry & fe, std::vector<char>& unpackData)
{
    uint8_t ror = (uint8_t)fe.checksum & 7;
    uint8_t key = (uint8_t)(fe.checksum >> 8);
    for (size_t i = 0; i<unpackData.size(); ++i)
    {
        unpackData[i] ^= key;
        unpackData[i] = unpackData[i] >> ror | unpackData[i] << (8 - ror);
    }
    return true;
}

bool deai5bu::DoExtractData(const file_entry & fe, std::vector<char>& unpackData)
{
    uint32_t key = 0x35353535;

    size_t dwordLen = unpackData.size() >> 2;
    uint32_t* ptr = (uint32_t*)unpackData.data();
    for (size_t i = 0; i<dwordLen; ++i)
        *ptr++ ^= key;

    size_t remain = unpackData.size() - (dwordLen << 2);
    for (; remain != 0; --remain)
    {
        unpackData[(dwordLen << 2) + remain - 1] ^= (uint8_t)(key >> (remain - 1));
    }
    return true;
}

bool colorfulcure::DoExtractData(const file_entry & fe, std::vector<char>& unpackData)
{
    xp3filter_decode("colorfulcure", fe.file_name, (uint8_t*)unpackData.data(), unpackData.size(), 0, unpackData.size(), fe.checksum);
    return true;
}

bool kamiyabai::DoExtractData(const file_entry & fe, std::vector<char>& unpackData)
{
    uint32_t key = 0xcdcdcdcd;

    size_t dwordLen = unpackData.size() >> 2;
    uint32_t* ptr = (uint32_t*)unpackData.data();
    for (size_t i = 0; i<dwordLen; ++i)
        *ptr++ ^= key;

    size_t remain = unpackData.size() - (dwordLen << 2);
    for (; remain != 0; --remain)
    {
        unpackData[(dwordLen << 2) + remain - 1] ^= (uint8_t)(key >> (remain - 1));
    }
    return true;
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
        wcscpy_s(fe.file_name, _countof(fe.file_name), name.c_str());
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
                {   // 不应该进来
                    assert(0);
                    AppendMsg(L"错误的文件索引记录\r\n");
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
                if (buf_size >= _countof(fe.file_name))
                {
                    MessageBox(0, L"文件名超出缓冲区长度\r\n", L"提示", MB_ICONWARNING | MB_OK);
                    buf_size = _countof(fe.file_name) - 1;
                }
                p += 0x2;
                memset(fe.file_name, 0, _countof(fe.file_name));
                memcpy(fe.file_name, (wchar_t*)p, buf_size * sizeof(wchar_t));

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

        wcscpy_s(Entry[i].file_name, _countof(Entry[i].file_name), nameList[n].file_name);
        ++n;
    }

    assert(n == nameList.size());
    return Entry;
}
