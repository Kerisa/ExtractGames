
#pragma once

#include <memory>
#include <string>
#include <vector>


class PfsFileImpl {
public:
    struct PfsEntry {
        std::string   mFileName;
        uint64_t      mOffset;
        uint32_t      mSize;
    };
    static constexpr uint32_t INDEX_START = 11;

public:
    virtual bool Open(const std::string& path) = 0;
    virtual bool ExtractIndeies() = 0;
    virtual bool ExtractResource() = 0;
};

class PfsFileImplV2 : public PfsFileImpl {
public:
    bool Open(const std::string& path) override;
    bool ExtractIndeies() override;
    bool ExtractResource() override;

    std::ifstream         mFileStream;
    std::string           mPath;
    std::string           mMagic;          // "pfX" 2/6/8
    uint32_t              mRelationFileDataOffset;
    uint32_t              mFileNumber;
    std::vector<PfsEntry> mEntries;
};

class PfsFileImplV6 : public PfsFileImpl {
public:
    bool Open(const std::string& path) override;
    bool ExtractIndeies() override;
    bool ExtractResource() override;

    std::ifstream         mFileStream;
    std::string           mPath;
    std::string           mMagic;          // "pfX" 2/6/8
    uint32_t              mRelationFileDataOffset;
    uint32_t              mFileNumber;
    std::vector<PfsEntry> mEntries;
};

class PfsFileImplV8 : public PfsFileImpl {
public:
    bool Open(const std::string& path) override;
    bool ExtractIndeies() override;
    bool ExtractResource() override;

    std::vector<char> Xor(const std::vector<char>& cipher);

    static constexpr uint32_t INDEX_HASH_START = 7;

    std::ifstream         mFileStream;
    std::string           mPath;
    std::string           mMagic;          // "pfX" 2/6/8
    uint32_t              mRelationFileDataOffset;
    uint32_t              mFileNumber;
    std::vector<PfsEntry> mEntries;
    std::vector<char>     mXorKey;
};


class PfsFile {
public:
    bool Open(const std::string& path);
    bool ExtractIndeies();
    bool ExtractResource();

    std::unique_ptr<PfsFileImpl> impl;
};
