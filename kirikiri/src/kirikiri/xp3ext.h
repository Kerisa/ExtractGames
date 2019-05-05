#pragma once

#include "xp3.h"



struct GameInfomation
{
    std::string GameID;
    std::function<EncryptedXP3*()> Handler;
};


extern std::map<std::wstring, GameInfomation> GameNameMap;
EncryptedXP3* CreateXP3Handler(const std::wstring& gameName);


extern void AppendMsg(const wchar_t *szBuffer);
void        XP3Entrance(const wchar_t *packName, const wchar_t *curDirectory, const std::wstring& choosedGame);


class kuranokunchi : public EncryptedXP3
{
public:
    bool DoExtractData(const file_entry& fe, std::vector<char>& unpackData) override;
};

class amakoi : public EncryptedXP3
{
public:
    bool DoExtractData(const file_entry& fe, std::vector<char>& unpackData) override;
};

class prettycation : public EncryptedXP3
{
public:
    bool DoExtractData(const file_entry& fe, std::vector<char>& unpackData) override;
};

class lovelycation : public EncryptedXP3
{
public:
    bool DoExtractData(const file_entry& fe, std::vector<char>& unpackData) override;
};

class swansong : public EncryptedXP3
{
public:
    bool DoExtractData(const file_entry& fe, std::vector<char>& unpackData) override;
};

class deai5bu : public EncryptedXP3
{
public:
    bool DoExtractData(const file_entry& fe, std::vector<char>& unpackData) override;
};

class kamiyabai : public EncryptedXP3
{
public:
    bool DoExtractData(const file_entry& fe, std::vector<char>& unpackData) override;
};


class colorfulcure : public EncryptedXP3
{
public:
    bool DoExtractData(const file_entry& fe, std::vector<char>& unpackData) override;
};

class sakurasaki : public EncryptedXP3
{
public:
    bool DoExtractData(const file_entry& fe, std::vector<char>& unpackData) override;
};



class anioka : public EncryptedXP3
{
public:
    bool DoExtractData(const file_entry& fe, std::vector<char>& unpackData) override;
};

class koisakura : public EncryptedXP3
{
public:
    bool DoExtractData(const file_entry& fe, std::vector<char>& unpackData) override;
};

class sukisuki : public EncryptedXP3
{
public:
    bool DoExtractData(const file_entry& fe, std::vector<char>& unpackData) override;
};

class Otomedomain : public EncryptedXP3
{
public:
    bool DoExtractData(const file_entry& fe, std::vector<char>& unpackData) override;
};

class oreaka : public EncryptedXP3
{
public:
    bool DoExtractData(const file_entry& fe, std::vector<char>& unpackData) override;
};