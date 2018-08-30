#pragma once

#include <fstream>
#include <string>
#include <vector>

class PAL
{
public:
    struct PACHeader
    {
        static constexpr uint32_t MAGIC = 0x20434150;  // "PAC "
        uint32_t Magic;
        uint32_t Reserved;
        uint32_t mEntryCount;
        char     UnkonwnBytes[0x7f8];
    };

    struct PACEntry
    {
        char        Filename[0x20];
        uint32_t    PackLength;
        uint32_t    Offset;
    };
    
    struct NormalizedEntry
    {
        enum TYPE {
            Base,
            Difference,
        };
        PACEntry    e;
        TYPE        Type;
        std::string Extra;
    };

public:
    bool Open(const std::string& path);
    void Close();
    bool ExtractEntries();
    bool ExtractResource(const std::string& saveDir);

private:
    void DecodeScript(std::vector<uint8_t>& data, size_t dataLength);

public:
    PACHeader                       mHeader;
    std::vector<NormalizedEntry>    mEntries;

private:
    std::ifstream mStream;
    std::string   mPath;
};