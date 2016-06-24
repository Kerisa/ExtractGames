
#pragma once

#include <memory>
#include <string>
#include <Windows.h>

class FileOperator
{
private:
    typedef HANDLE FilePtr;

public:
    typedef unsigned char U8;
    typedef unsigned short U16;
    typedef unsigned long U32;
    typedef unsigned long long U64;

    typedef U8* PU8;
    typedef U16* PU16;
    typedef U32* PU32;
    typedef U64* PU64;

    enum MoveMethod {
        _FILE_BEGIN,
        _FILE_CURRENT,
        _FILE_END,
    };
    
public:
    FileOperator();
    ~FileOperator();

    // Read Operator
    bool OpenNewFile(std::wstring &filename);
    bool ReadFile(void *ptr, U32 bytesToRead, PU32 pbytesRead = nullptr);
    std::shared_ptr<U8> ReadFileBlock(U32 bytesToRead, PU32 pbytesRead = nullptr);
    bool IsFileOpened() const;
    U64 GetFileSize() const;
    void CloseReadFile();

    // Write Operator
    bool IsFileCreated() const;
    void CloseWriteFile();
    bool SaveAsFile(void *_data, U32 len, const std::wstring &filename, const std::wstring &directory = std::wstring());
    bool SaveAsFile(void *_data, U32 len, std::string &filename, std::wstring &directory = std::wstring());


    bool SetFilePointer(U64 offset, MoveMethod m);
    const std::wstring & GetCurFileName() const;

private:
    template <class T>
    T Min(T a, T b);

    void CloseFile(FilePtr & f);

    bool pSetFilePointer(U64 offset, MoveMethod m, PU64 newOffset);

    bool CreateNewFileToWrite(const std::wstring &filename, const std::wstring & directory);
    bool WriteFile(void *data, U32 len, PU32 pbytesWrited = nullptr);
    void CreatePrepare(const std::wstring & filename, const std::wstring & directory, std::wstring & fn);

private:
    FilePtr pFileRead, pFileWrite;
    std::wstring FileName;
};

template<class T>
inline T FileOperator::Min(T a, T b)
{
    return a < b ? a : b;
}
