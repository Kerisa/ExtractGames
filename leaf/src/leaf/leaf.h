#pragma once

#include <fstream>
#include <string>
#include <vector>

class Leaf
{
public:
    struct Header {
        enum {
            kLacMagic  = 0x43414c,
            kKcapMagic = 0x5041434b,

            kLacSize   = 8,
            kKcapSize  = 16,
        };
        uint32_t magic{ 0 };  // 0x5041434b
        uint32_t unknown1{ 0 };
        uint32_t unknown2{ 0 };
        uint32_t entry_number{ 0 };
    };

    struct Entry {
        uint32_t    is_compressed{ 0 };
        std::string name;
        uint32_t    unknown2{ 0 };
        uint32_t    unknown3{ 0 };
        uint32_t    offset{ 0 };
        uint32_t    pack_size{ 0 };
    };

    bool Open(const std::string& file);
    bool ExtractEntries();
    bool ExtractResource();

    std::string JPtoGBK(const std::string& s);
    int  Decompress(char* output, char* input, int input_size, int output_size);

    std::string        mPath;
    std::ifstream      mPackage;
    Header             mHeader;
    std::vector<Entry> mEntries;
};