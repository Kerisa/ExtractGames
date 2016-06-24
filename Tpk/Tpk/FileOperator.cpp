
#include <assert.h>
#include "FileOperator.h"

FileOperator::FileOperator()
{
    pFileRead = INVALID_HANDLE_VALUE;
    pFileWrite = INVALID_HANDLE_VALUE;
}

FileOperator::~FileOperator()
{
    if (IsFileOpened())
        CloseReadFile();
    if (IsFileCreated())
        CloseWriteFile();
}

bool FileOperator::OpenNewFile(std::wstring &filename)
{
    if (IsFileOpened())
        CloseReadFile();

    pFileRead = ::CreateFileW(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    
    return IsFileOpened();
}

bool FileOperator::ReadFile(void * ptr, U32 bytesToRead, PU32 pbytesRead)
{
    if (ptr == nullptr || !IsFileOpened())
        return false;

    U32 dummy;
    return ::ReadFile(pFileRead, ptr, bytesToRead, pbytesRead ? pbytesRead : &dummy, 0);
}

std::shared_ptr<FileOperator::U8> FileOperator::ReadFileBlock(U32 bytesToRead, PU32 pbytesRead)
{
    if (!IsFileOpened())
        return std::shared_ptr<FileOperator::U8>();

    U64 curOffset;
    pSetFilePointer(0, _FILE_CURRENT, &curOffset);

    U32 bytesCanRead = Min(static_cast<U32>(GetFileSize() - curOffset), bytesToRead);

    std::shared_ptr<U8> data(new U8[bytesCanRead]);
    assert(data.use_count() != 0);
    memset(data.get(), 0, bytesCanRead);

    U32 dummy;
    bool b = ::ReadFile(pFileRead, &(*data), bytesCanRead, pbytesRead ? pbytesRead : &dummy, 0);
    //assert(b && GetLastError() == 0);

    return data;
}

bool FileOperator::SetFilePointer(U64 offset, MoveMethod m)
{
    if (!IsFileOpened())
        return false;

    return pSetFilePointer(offset, m, 0);
}

bool FileOperator::IsFileOpened() const
{
    return pFileRead != INVALID_HANDLE_VALUE;
}

void FileOperator::CloseFile(FilePtr & f)
{
    ::CloseHandle(f);
    f = INVALID_HANDLE_VALUE;
}

const std::wstring & FileOperator::GetCurFileName() const
{
    return FileName;
}

FileOperator::U64 FileOperator::GetFileSize() const
{
    if (!IsFileOpened())
        return 0;

    ULARGE_INTEGER li;
    li.LowPart = ::GetFileSize(pFileRead, &li.HighPart);
    return li.QuadPart;
}

void FileOperator::CloseReadFile()
{
    CloseFile(pFileRead);
}

bool FileOperator::IsFileCreated() const
{
    return pFileWrite != INVALID_HANDLE_VALUE;
}

void FileOperator::CloseWriteFile()
{
    CloseFile(pFileWrite);
}

bool FileOperator::SaveAsFile(void * _data, U32 len, const std::wstring & filename, const std::wstring & directory)
{

    bool c = CreateNewFileToWrite(filename, directory);

    bool ret =  c ? WriteFile(_data, len) : false;
    CloseWriteFile();

    return ret;
}

bool FileOperator::SaveAsFile(void * _data, U32 len, std::string & filename, std::wstring & directory)
{
    U32 size = (filename.size() + 2) * sizeof(wchar_t);
    wchar_t * w = (wchar_t*)malloc(size);
    memset(w, 0, size);
    MultiByteToWideChar(CP_ACP, 0, filename.c_str(), -1, w, filename.size());
    std::wstring ws(w);
    delete[] w;

    return SaveAsFile(_data, len, ws, directory);
}

bool FileOperator::pSetFilePointer(U64 offset, MoveMethod m, PU64 newOffset)
{
    if (!IsFileOpened())
        return false;

    LARGE_INTEGER li, nli;
    li.QuadPart = offset;
    bool ret = ::SetFilePointerEx(pFileRead, li, &nli, m);

    if (newOffset)
    {
        if (ret)
            *newOffset = nli.QuadPart;
        else
            *newOffset = 0;
    }

    return ret;
}

bool FileOperator::CreateNewFileToWrite(
    const std::wstring & filename,
    const std::wstring & directory
    )
{
    if (IsFileCreated())
        CloseWriteFile();

    std::wstring fn;
    CreatePrepare(filename, directory, fn);

    pFileWrite = ::CreateFileW(fn.c_str(),
        GENERIC_WRITE, 0, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);

    return IsFileCreated();
}

bool FileOperator::WriteFile(void * data, U32 len, PU32 pbytesWrited)
{
    if (!data || !IsFileCreated())
        return false;

    U32 dummy;
    return ::WriteFile(pFileWrite, data, len, pbytesWrited ? pbytesWrited : &dummy, 0);
}

void FileOperator::CreatePrepare(
    const std::wstring & filename,
    const std::wstring & directory,
    std::wstring & fn
    )
{
    fn.assign(directory);
    if (fn.rbegin() != fn.rend() && *(fn.rbegin()) != L'\\')
        fn += L'\\';

    for (int i = 0; i < filename.size(); ++i)
        if (filename[i] == L'/')
            fn += L'\\';
        else
            fn += filename[i];

    size_t pos = 0;
    while (pos < fn.size())
    {
        pos = fn.find(L'\\', pos+1);
        if (pos == std::wstring::npos)
            break;

        std::wstring dir(fn.substr(0, pos));
        ::CreateDirectoryW(dir.c_str(), 0);
    }
}

