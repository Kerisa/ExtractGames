
#include "xp3ext.h"
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

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

bool colorfulcure::DoExtractData(const file_entry & fe, std::vector<char>& unpackData)
{
    xp3filter_decode("colorfulcure", fe.file_name.c_str(), (uint8_t*)unpackData.data(), unpackData.size(), 0, unpackData.size(), fe.checksum);
    return true;
}

bool sakurasaki::DoExtractData(const file_entry & fe, std::vector<char>& unpackData)
{
    xp3filter_decode("sakurasaki", fe.file_name.c_str(), (uint8_t*)unpackData.data(), unpackData.size(), 0, unpackData.size(), fe.checksum);
    return true;
}



///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////






void XP3Entrance(const wchar_t *packName, const wchar_t *curDirectory, const std::wstring& choosedGame)
{
    DWORD idx_size = 0;

    auto fucker = CreateXP3Handler(choosedGame);
    if (!fucker)
    {
        AppendMsg(wstring(L"无法处理的游戏[" + choosedGame + L"]\r\n").c_str());
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
    int saveFileCount = fucker->ExtractData(entries, curDirectory, wcout);
    fucker->Close();
    delete fucker;

    if (entries.size() == saveFileCount)
    {
        wstringstream wss;
        wss << L"[提取完成(" << saveFileCount << L"/" << saveFileCount << L")]" << packName << "\r\n";
        AppendMsg(wss.str().c_str());
    }
    else
    {
        wstringstream wss;
        wss << L"提取" << saveFileCount << L"个文件，共" << entries.size() << L"个，有"
            << entries.size() - saveFileCount << L"个发生错误\r\n" << packName << L"\r\n";
        MessageBoxW(0, wss.str().c_str(), L"提示", MB_ICONWARNING);
    }
}

std::map<std::wstring, GameInfomation> GameNameMap = {
    { L"<默认：直接提取>",{ "Unencrypted", []() { return new EncryptedXP3; } } },
    { L"さくら、咲きました。",{ "sakurasaki", []() { return new sakurasaki; } } },
    { L"倉野くんちのふたご事情",{ "kuranokunchi", []() { return new kuranokunchi; } } },
    { L"あま恋シロップス ～恥じらう恋心でシたくなる甘神様の恋祭り～",{ "amakoi", []() { return new amakoi; } } },
    { L"カミツレ ～7の二乗不思議～",{ "Unencrypted", []() { return new EncryptedXP3; } } },
    { L"すたーらいと★アイドル -COLORFUL TOP STAGE！-",{ "Unencrypted", []() { return new EncryptedXP3; } } },
    { L"あまたらすリドルスター",{ "Unencrypted", []() { return new EncryptedXP3; } } },
    { L"カラフル☆きゅあ～ 缤纷少女",{ "colorfulcure", []() { return new colorfulcure; } } },
    { L"ＰＲＥＴＴＹ×Ｃ∧ＴＩＯＮ",{ "prettycation", []() { return new prettycation; } } },
    { L"your diary ＋H",{ "kuranokunchi", []() { return new kuranokunchi; } } },
    { L"迷える2人とセカイのすべて",{ "Unencrypted", []() { return new EncryptedXP3; } } },
    { L"迷える2人とセカイのすべて LOVEHEAVEN300％",{ "Unencrypted", []() { return new EncryptedXP3; } } },
    { L"ヤリまん娘 ～俺の妹はビチビチビッチ～",{ "Unencrypted", []() { return new EncryptedXP3; } } },
    { L"公園いたずらシミュレータ ver.MAKO",{ "Unencrypted", []() { return new EncryptedXP3; } } },
    { L"PRETTY×CATION2",{ "prettycation", []() { return new prettycation; } } },
    { L"エロ本を捨ててから兄の様子がおかしい",{ "anioka", []() { return nullptr; } } },
    { L"SWANSONG",{ "swansong", []() { return new swansong; } } },
    { L"恋がさくころ桜どき",{ "koisakura", []() { return nullptr; } } },
    { L"ずっとすきして たくさんすきして",{ "sukisuki", []() { return nullptr; } } },
    { L"俺と5人の嫁さんがラブラブなのは、未来からきた赤ちゃんのおかげに違いない！？",{ "oreaka", []() { return nullptr; } } },
    { L"妹のセイイキ(未完成)",{ "seiiki", []() { return nullptr; } } },
    { L"オトメ＊ドメイン",{ "Otomedomain", []() { return nullptr; } } },
    { L"LOVELY×CATION2",{ "lovelycation", []() { return new lovelycation; } } },
    { L"出会って5分は俺のもの！ 時間停止と不可避な運命",{ "deai5bu", []() { return new deai5bu; } } },
    { L"神頼みしすぎて俺の未来がヤバい。",{ "kamiyabai", []() { return new kamiyabai; } } },
    { L"9-nine-ここのつここのかここのいろ",{ "palette 9-nine", []() { return new palette_9_nine; } } },
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
