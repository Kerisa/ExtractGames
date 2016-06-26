
#include <assert.h>
#include "FileOperator.h"

FileOperatorCommon::FileOperatorCommon()
{
    m_File = INVALID_HANDLE_VALUE;
}

FileOperatorCommon::~FileOperatorCommon()
{
    if (IsFileValid())
        Close();
}

bool FileOperatorCommon::IsFileValid() const
{
    return m_File != INVALID_HANDLE_VALUE;
}

U64 FileOperatorCommon::GetFileSize() const
{
    if (!IsFileValid())
        return 0;

    ULARGE_INTEGER li;
    li.LowPart = ::GetFileSize(m_File, &li.HighPart);
    return li.QuadPart;
}

bool FileOperatorCommon::SetFilePointer(U64 offset, MoveMethod m, PU64 newOffset)
{
    if (!IsFileValid())
        return false;

    LARGE_INTEGER li, nli;
    li.QuadPart = offset;
    bool ret = ::SetFilePointerEx(m_File, li, &nli, m);

    if (newOffset)
    {
        if (ret)
            *newOffset = nli.QuadPart;
        else
            *newOffset = 0;
    }

    return ret;
}

void FileOperatorCommon::Close()
{
    ::CloseHandle(m_File);
    m_File = INVALID_HANDLE_VALUE;
    m_FileName.clear();
}




bool ReadOperator::Assign(const std::wstring &filename)
{
    if (IsFileValid())
        Close();

    m_File = ::CreateFileW(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    
    bool ret = IsFileValid();
    if (ret)
        m_FileName.assign(filename);

    return ret;
}

bool ReadOperator::ReadFile(void * ptr, U32 bytesToRead, PU32 pbytesRead)
{
    if (ptr == nullptr || !IsFileValid())
        return false;

    U32 dummy;
    return ::ReadFile(m_File, ptr, bytesToRead, pbytesRead ? pbytesRead : &dummy, 0);
}

std::shared_ptr<U8> ReadOperator::ReadFileBlock(U32 bytesToRead, PU32 pbytesRead)
{
    if (!IsFileValid())
        return std::shared_ptr<U8>();

    U64 curOffset;
    FileOperatorCommon::SetFilePointer(0, _FILE_CURRENT, &curOffset);

    U32 bytesCanRead = Min(static_cast<U32>(GetFileSize() - curOffset), bytesToRead);

    std::shared_ptr<U8> data(new U8[bytesCanRead]);
    memset(data.get(), 0, bytesCanRead);

    U32 dummy;
    bool b = ::ReadFile(m_File, &(*data), bytesCanRead, pbytesRead ? pbytesRead : &dummy, 0);

    return data;
}




bool WriteOperator::Assign(const std::wstring & filename, bool forceNew)
{
    if (IsFileValid())
        Close();
    
    if (forceNew)
        m_File = ::CreateFileW(filename.c_str(),
            GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    else
        m_File = ::CreateFileW(filename.c_str(),
            GENERIC_WRITE, 0, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);

    bool ret = IsFileValid();
    if (ret)
        m_FileName.assign(filename);

    return ret;
}


bool WriteOperator::SaveAsFile(void * _data, U32 len, const std::wstring & filename)
{
    bool c = Assign(filename);

    bool ret =  c ? WriteFile(_data, len) : false;
    Close();

    return ret;
}

bool WriteOperator::WriteFile(void * data, U32 len, PU32 pbytesWrited)
{
    if (!data || !IsFileValid())
        return false;

    U32 dummy;
    return ::WriteFile(m_File, data, len, pbytesWrited ? pbytesWrited : &dummy, 0);
}


