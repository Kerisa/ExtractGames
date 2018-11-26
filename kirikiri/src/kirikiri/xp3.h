

#pragma once


#include <Windows.h>
#include <fstream>
#include <vector>
#include <string>
#include <assert.h>
#include "error.h"
#include "cxdec\cxdec.h"
#include "xp3filter_decode.h"
#include <strsafe.h>

typedef int(*UNCOMPRESS)(char* dst, uint32_t* dstLength, char* src, uint32_t srcLength);
extern UNCOMPRESS unCom;

#pragma pack(1)
struct xp3_file_header
{
	BYTE magic[11]; // = {'\x58', '\x50', '\x33', '\x0D', '\x0A', '\x20', '\0A', '\x1A', '\x8B', '\x67', '\x01'};
	uint64_t offset;
	DWORD minor_version;	// 1
	BYTE flag;	// 0x80 TVP_XP3_INDEX_CONTINUE
	uint64_t index_size;
	uint64_t index_offset;
};
#pragma pack()


struct file_entry
{
    enum {
        StrCapacity = 128,
        Section = 16
    };
	DWORD checksum;
	DWORD encryption_flag;	// info
    int part;
    struct {
        DWORD compress_flag;		// segm
        uint64_t offset;
        uint64_t orig_length;
        uint64_t pkg_length;
    } info[Section];
    wchar_t file_name[StrCapacity];
};


extern void AppendMsg(const wchar_t *szBuffer);


DWORD   HaveExtraEntryChunk(const char *game);
int     XP3ArcPraseEntryStage1(PVOID _idx, DWORD _len, std::vector<file_entry>& Entry, DWORD chunk);
void    XP3Entrance(const wchar_t *packName, const wchar_t *curDirectory, const std::string& choosedGame);




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
public:
    ~EncryptedXP3();

    bool Open(const std::wstring& path);
    void Close();
    bool IsValid();

    std::vector<char> GetPlainIndexBytes();
    std::vector<file_entry> XP3ArcPraseEntryStage0(const std::vector<char>& plainBytes);
    int ExtractData(const std::vector<file_entry>& entries, const std::wstring& saveDir);

    int SplitFileNameAndSave(const std::wstring& cur_dir, const std::wstring& file_name, const std::vector<char>& unpackData);

protected:
    virtual bool DoExtractData(const file_entry& fe, std::vector<char>& unpackData);

private:
    std::wstring mPath;
    std::ifstream mStream;
};

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


class colorfulcure : public EncryptedXP3
{
public:
    bool DoExtractData(const file_entry& fe, std::vector<char>& unpackData) override;
};

EncryptedXP3* CreateXP3Handler(const std::string& gameName);