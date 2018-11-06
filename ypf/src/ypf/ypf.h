#pragma once

#include <fstream>
#include <string>
#include <vector>

class YPF
{
public:
    struct YpfHeader
    {
        static constexpr uint32_t MAGIC = '\0FPY';
        uint32_t mMagic;
        uint32_t mUnknown1;
        uint32_t mFileCount;
        uint32_t mDataStart;
        uint8_t  mZero[16];
    };

    struct YpfEntry
    {
        uint8_t unknown[3];
        uint8_t name_length_correct;
        uint8_t name_length;     // 要解码
        uint8_t name[1];         // 要解码
        uint8_t filetype;
        uint8_t zlib_compress_flag;
        uint32_t original_length;
        uint32_t compressd_length;
        uint32_t offset;
        uint32_t unknwon;      // 估计是 crc / alder32 之类的
    };

    struct NormalizedEntry
    {
        std::string filename;
        uint8_t filetype;
        bool zlib_compressed{ false };
        uint32_t original_length;
        uint32_t compressd_length;
        uint32_t offset;
    };

public:
    bool Open(const std::string& path);
    void Close();
    bool ExtractEntries();
    bool ExtractResource(const std::string& saveDir);

    std::string GetFolderOfPath(const std::string& fullPath);
    std::string ConvJPToChs(const std::string& str);

private:
    std::vector<NormalizedEntry>    mEntries;
    std::string                        mPath;
    std::ifstream                    mStream;
    YpfHeader                        mHeader;
};