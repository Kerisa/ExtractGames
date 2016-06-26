
#pragma once

#include <assert.h>
#include <memory>
#include <string>
#include <Windows.h>
#include "type.h"

class FileOperatorCommon
{
public:
    typedef HANDLE FilePtr;
    
    enum MoveMethod {
        _FILE_BEGIN,
        _FILE_CURRENT,
        _FILE_END,
    };

    FileOperatorCommon();
    ~FileOperatorCommon();
    
    virtual bool Assign(const std::wstring & filename) { assert(0); return false; }
    bool IsFileValid() const;
    U64 GetFileSize() const;
    bool SetFilePointer(U64 offset, MoveMethod m, PU64 newOffset = 0);
    void Close();


    template <class T>
    T Min(T a, T b);

protected:
    FilePtr m_File;
    std::wstring m_FileName;
};


class ReadOperator : public FileOperatorCommon
{
public:
    bool Assign(const std::wstring & filename);
    bool ReadFile(void *ptr, U32 bytesToRead, PU32 pbytesRead = nullptr);
    std::shared_ptr<U8> ReadFileBlock(U32 bytesToRead, PU32 pbytesRead = nullptr);
};


class WriteOperator : public FileOperatorCommon
{
public:
    bool Assign(const std::wstring & filename, bool forceNew = false);
    //bool CreateNewFileToWrite(const std::wstring &filename, const std::wstring & directory);
    bool WriteFile(void *data, U32 len, PU32 pbytesWrited = nullptr);

    bool SaveAsFile(void *_data, U32 len, const std::wstring &filename);
};


template<class T>
inline T FileOperatorCommon::Min(T a, T b)
{
    return a < b ? a : b;
}
