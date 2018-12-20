

#pragma once

#include <array>
#include <cassert>
#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <strsafe.h>
#include <Windows.h>
#include "cxdec\cxdec.h"
#include "xp3filter_decode.h"

typedef int(*UNCOMPRESS)(char* dst, uint32_t* dstLength, char* src, uint32_t srcLength);
extern UNCOMPRESS unCom;

#pragma pack(1)
struct xp3_file_header
{
    BYTE magic[11]; // = {'\x58', '\x50', '\x33', '\x0D', '\x0A', '\x20', '\0A', '\x1A', '\x8B', '\x67', '\x01'};
    uint64_t offset;
    DWORD minor_version;    // 1
    BYTE flag;    // 0x80 TVP_XP3_INDEX_CONTINUE
    uint64_t index_size;
    uint64_t index_offset;
};
#pragma pack()


#pragma pack (push,1)
struct FileSection
{
    static constexpr uint32_t MAGIC = 'eliF';
    uint32_t Magic;
    uint64_t SizeOfData;
};

struct TimeSection
{
    static constexpr uint32_t MAGIC = 'emit';
    uint32_t Magic;
    uint64_t SizeOfData;
    uint64_t Time;
};

struct AdlrSection
{
    static constexpr uint32_t MAGIC = 'rlda';
    uint32_t Magic;
    uint64_t SizeOfData;
    uint32_t Checksum;
};

struct SegmSection
{
    static constexpr uint32_t MAGIC = 'mges';
    uint32_t Magic;
    uint64_t SizeOfData;
    uint32_t IsCompressed;      // 0x1
    uint64_t Offset;
    uint64_t OriginalSize;
    uint64_t PackedSize;
};

struct InfoSection
{
    static constexpr uint32_t MAGIC = 'ofni';
    uint32_t Magic;
    uint64_t SizeOfData;
    uint32_t EncryptFlag;       // 0x80000000
    uint64_t OriginalSize;
    uint64_t PackedSize;
    uint16_t NameInWords;
    wchar_t  NamePtr[1];
};

struct ExtraSection
{
    uint32_t Magic{ 0 };
    uint64_t SizeOfData{ 0 };
    uint32_t Checksum{ 0 };
    uint16_t NameInWords;
    wchar_t  NamePtr[1];
};

#pragma pack(pop)

struct file_entry
{
    DWORD checksum{ 0 };
    DWORD encryption_flag{ 0 };    // info
    std::vector<SegmSection> mInfo;
    std::wstring file_name;
    std::wstring internal_name;
    ExtraSection mExtra;
    uint64_t mFileTime{ 0 };

    uint32_t GetTotlePackedSize() const;
    uint32_t GetTotleOriginalSize() const;
    bool     IsCompressed() const;
    bool     IsEncrypted() const;
    bool     ReadFileData(std::ifstream& file, std::vector<char>& packedData) const;

};




DWORD   HaveExtraEntryChunk(const char *game);
int     XP3ArcPraseEntryStage1(PVOID _idx, DWORD _len, std::vector<file_entry>& Entry, DWORD chunk);
void    XP3Entrance(const wchar_t *packName, const wchar_t *curDirectory, const std::wstring& choosedGame);




static const struct _XP3ENTRYEXTRACHUNK {
    char *name;
    DWORD chunk;
}
XP3EntryExtraChunk[] = {
    "seiiki",   0x676e6566,
    "nekopara", 0x0,
};



class EncryptedXP3
{
    static UNCOMPRESS unCom;
    static constexpr uint32_t MagicHnfn = 'nfnh';
    static constexpr uint32_t MagicFeng = 'gnef';

public:
    EncryptedXP3();
    virtual ~EncryptedXP3();

    bool Open(const std::wstring& path);
    void Close();
    bool IsValid();

    std::vector<char> GetPlainIndexBytes();
    void DumpEntriesToFile(const std::vector<file_entry>& entries, const std::wstring& path);
    std::vector<file_entry> ExtractEntries(const std::vector<char>& plainBytes);
    int ExtractData(const std::vector<file_entry>& entries, const std::wstring& saveDir, std::wostream& output);

    static int SplitFileNameAndSave(const std::wstring& cur_dir, const std::wstring& file_name, const std::vector<char>& unpackData);
    std::wstring FormatFileNameForIStream(const file_entry& fe) const;
    bool ReadEntryDataOfAllParts(const file_entry& fe, std::vector<char>& packedData, uint32_t* pOriginalLength);

protected:
    virtual bool DoExtractData(const file_entry& fe, std::vector<char>& unpackData);
    virtual std::vector<file_entry> XP3ArcPraseEntryStage0(uint32_t extraMagic, const std::vector<char>& plainBytes);
    std::vector<file_entry> ParsePalette_9nine(const std::vector<char>& plainBytes);

    bool ParseFileSection(const uint8_t* ptr, uint32_t* secSize, uint32_t* entrySize);
    bool ParseSegmSection(const uint8_t* ptr, file_entry& fe, uint32_t* secSize);
    bool ParseInfoSection(const uint8_t* ptr, file_entry& fe, uint32_t* secSize);
    bool ParseAdlrSection(const uint8_t* ptr, file_entry& fe, uint32_t* secSize);
    bool ParseTimeSection(const uint8_t* ptr, file_entry& fe, uint32_t* secSize);
    bool ParseExtraSection(const uint8_t* ptr, uint32_t extraMagic, file_entry& fe, uint32_t* secSize);
    bool ParseProtectWarning(const uint8_t* ptr, uint32_t* secSize);
    bool HasExtraSection(const std::vector<char>& plainBytes, uint32_t* magic);

private:
    xp3_file_header mHeader;
    std::wstring mPath;
    std::ifstream mStream;
    uint32_t mExtraSectionMagic{ 0 };
};

class palette_9_nine : public EncryptedXP3
{
public:
    std::vector<file_entry> XP3ArcPraseEntryStage0(const std::vector<char>& plainBytes);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


