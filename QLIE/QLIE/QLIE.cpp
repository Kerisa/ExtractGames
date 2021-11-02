
#include <cassert>
#include <sstream>
#include <Windows.h>
#include <strsafe.h>
#include "QLIE.h"

using namespace std;


int Mangekyoo_1_2_3::SplitFileNameAndSave(const wchar_t *cur_dir, const wchar_t *file_name, void* unpack, unsigned long file_length)
{
    wstring buf = wstring(cur_dir) + L"\\" + file_name;
    size_t i = wcslen(cur_dir) + 1;
    const wchar_t *p = buf.c_str(), *end = buf.c_str() + buf.size();
    while (p <= end && i < buf.size())
    {
        while (buf[i] != '\\' && buf[i] != '/' && buf[i] != '\0') ++i;
        if (buf[i] == '/') buf[i] = '\\';
        if (i < buf.size())
        {
            wchar_t tmp = buf[i];
            buf[i] = '\0';

            CreateDirectoryW(p, 0);
            buf[i] = tmp;
            ++i;
        }
    }

    HANDLE hFile;
    int ret = 0;
    wstringstream wss;
    do {
        hFile = CreateFile(buf.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            wss << L"[文件创建错误] " << file_name << L"\r\n";
            ret = -1;
            break;
        }

        DWORD ByteWrite;
        WriteFile(hFile, unpack, file_length, &ByteWrite, NULL);

        if (ByteWrite != file_length)
        {
            wss << L"[文件写入错误] " << file_name << L"\r\n";
            ret = -2;
            break;
        }

        int t = GetLastError();
        if (!t || t == ERROR_ALREADY_EXISTS)
        {
            wss << L"[已保存]" << file_name << L"\r\n";
        }
        else
        {
            wss << L"[无法保存]" << file_name << L"\r\n";
            ret = -3;
        }
    } while (0);

    AppendMsg(wss.str().c_str());
    CloseHandle(hFile);
    return ret;
}


uint32_t Mangekyoo_1_2_3::GenerateKey(PVOID Buf, uint32_t Len)
{
    // paddw
    PWORD wBuf = (PWORD) Buf;
    WORD  mm0[4], mm2[4];

    for (uint32_t i=0; i<4; ++i)
        mm0[i] = mm2[i] = 0;

    for (uint32_t k=0; k<Len/8; ++k)
    {
        for (uint32_t i=0; i<4; ++i)
        {
            mm2[i] += 0x307;
            mm0[i] += wBuf[i] ^ mm2[i];
        }
        wBuf += 4;
    }

    return (mm0[3] ^ mm0[1]) << 16 | mm0[0] ^ mm0[2];
}


void Mangekyoo_1_2_3::Decode(PVOID Buf, uint32_t Len, uint32_t Key)
{
    uint32_t* dBuf = (uint32_t*) Buf;
    uint32_t  mm5[2], mm6[2], mm7[2];

    for (uint32_t i=0; i<2; ++i)
    {
        mm6[i] = 0xce24f523;
        mm7[i] = 0xa73c5f9d;
        mm5[i] = (Key + Len) ^ 0xfec9753e;
    }

    for (uint32_t i=0; i<Len/8; ++i)
    {
        for (uint32_t j=0; j<2; ++j)
        {
            mm7[j] = (mm7[j] + mm6[j]) ^ mm5[j];
            dBuf[j] ^= mm7[j];
            mm5[j] = dBuf[j];
        }
        dBuf += 2;
    }
    return;
}


int Mangekyoo_1_2_3::Decode_Sub1(uint32_t dwSeed, uint32_t* arrDecode, uint32_t* pSubFlag)
{
    // 类似于初始化

    arrDecode[0] = dwSeed & 0xffffffff;

    for (uint32_t i=0; i<63; ++i)
    {
        arrDecode[i+1] = (uint32_t)((arrDecode[i] ^ (arrDecode[i] >> 30)) * 0x6611bc19) + i + 1;
    }

    *pSubFlag = 1;

    return 0;
}


void Mangekyoo_1_2_3::Decode_Sub2(PVOID Buf, uint32_t Len, uint32_t* arrDecode)
{
    // 处理exe文件中的Key时长度是0x423e，
    // 但好像无所谓反正只用前面256字节

    uint32_t* pdwBuf = (uint32_t*) Buf;
    uint32_t  Len1 = Len / 4;

    if (Len1 > 64)
        Len1 = 64;

    for (uint32_t i=0; i<Len1; ++i)
    {
        arrDecode[i] ^= pdwBuf[i];
    }

    return;
}


uint32_t Mangekyoo_1_2_3::Decode_Sub3(uint32_t* arrDecode, uint32_t* pSubFlag)
{
    static const uint32_t A = 64, B = 39;
    uint32_t i, eax;


    if (0 == --(*pSubFlag))
    {
        *pSubFlag = A;

        for (i=0; i<A-B; ++i)
        {
            eax = (arrDecode[i] & 0x80000000) | ((arrDecode[i+1] & 0x7fffffff) >> 1);
            if (arrDecode[i+1] & 1)
                eax ^= 0x9908b0df;
            arrDecode[i] = eax ^ arrDecode[i+B];
        }

        for (; i<A-1; ++i)
        {
            eax = (arrDecode[i] & 0x80000000) | ((arrDecode[i+1] & 0x7fffffff) >> 1);
            if (arrDecode[i+1] & 1)
                eax ^= 0x9908b0df;
            arrDecode[i] = eax ^ arrDecode[i-(A-B)];
        }

        eax = (arrDecode[A-1] & 0x80000000) | ((arrDecode[0] & 0x7fffffff) >> 1);
        if (arrDecode[A] & 1)			//	!
            eax ^= 0x9908b0df;
        arrDecode[A-1] = eax ^ arrDecode[B-1];
    }

    eax = arrDecode[A - *pSubFlag];		//	!

    eax ^= eax >> 11;
    eax ^= (eax << 7) & 0x9c4f88e3;
    eax ^= (eax << 15) & 0xe7f70000;
    eax ^= eax >> 18;

    return eax;
}


int Mangekyoo_1_2_3::DecodeFileData(PVOID PackData, const PACKIDX& Idx, uint8_t* pExeKey, uint32_t dwExeKeyLen, uint8_t* pKeyFile, uint32_t dwKeyFileLen)
{
    uint32_t dwSum = 0x85f532, dwSeed = 0x33f641, dwFileNameLen;
    uint32_t arr1Idx, SubFlag;
    uint32_t arrDecode[256];	// "8qH"

    dwFileNameLen = Idx.Name.size();
    for (uint32_t i=0; i<dwFileNameLen; ++i)
    {
        dwSum  += (BYTE)Idx.Name[i] * (BYTE)i;		// Name要无符号啊！ 妈蛋
        dwSeed ^= dwSum;
    }

    dwSeed += ((Idx.CompressLen ^ 0x8f32dc ^ dwSum) + dwSum + Idx.CompressLen + 7 * (Idx.CompressLen & 0xffffff)) ^ Idx.Key;
    dwSeed =  (dwSeed & 0xffffff) * 9;
    dwSeed ^= 0x453a;

    uint32_t arrDecode_1[256];
    memset(arrDecode_1, 0, 1024);

    Decode_Sub1(dwSeed, arrDecode, &SubFlag);

    Decode_Sub2(pKeyFile, dwKeyFileLen, arrDecode);
    Decode_Sub2(pExeKey, dwExeKeyLen, arrDecode);

    for (uint32_t i=0; i<0x29; ++i)
        arrDecode_1[i] = Decode_Sub3(arrDecode, &SubFlag);

    arrDecode_1[0x29] = 0;


    typedef union {
        ULONGLONG	Qword;
        uint32_t		uint32_t[2];
        WORD		Word[4];
        BYTE		Byte[8];
    } UQWORD;
    UQWORD Tmp;
    Tmp.uint32_t[0] = Decode_Sub3(arrDecode, &SubFlag);
    Tmp.uint32_t[1] = Decode_Sub3(arrDecode, &SubFlag);

    ULONGLONG *p     = (ULONGLONG*) PackData;
    ULONGLONG *pEnd  = (ULONGLONG*) PackData + Idx.CompressLen / 8;
    ULONGLONG *parr1 = (ULONGLONG*) arrDecode_1;

    arr1Idx = Decode_Sub3(arrDecode, &SubFlag) & 0xf;
    while (p < pEnd)
    {
        // pxor mm7, mm6
        Tmp.Qword ^= *(parr1 + arr1Idx);

        // paddd mm7, mm6
        Tmp.uint32_t[0] += * (uint32_t*)(parr1 + arr1Idx)     ;
        Tmp.uint32_t[1] += *((uint32_t*)(parr1 + arr1Idx) + 1);

        // movq mm0, qword ptr[edi]
        // pxor mm0, mm7
        // movq mm1, mm0
        // mov1 qword ptr [edi], mm0
        *p ^= Tmp.Qword;

        // paddb mm7, mm1   (*ppd)
        for (uint32_t i=0; i<8; ++i)
            Tmp.Byte[i] += *((uint8_t*)p + i);

        // pxor mm7, mm1
        Tmp.Qword ^= *p;

        // pslld mm7, 1
        Tmp.uint32_t[0] <<= 1;
        Tmp.uint32_t[1] <<= 1;

        // paddw mm7, mm1
        for (uint32_t i=0; i<4; ++i)
            Tmp.Word[i] += *((PWORD)p + i);

        ++p;
        arr1Idx = ++arr1Idx & 0xf;
    }



    return 0;
}


int Mangekyoo_1_2_3::Decompress(PVOID HashData, uint32_t HashDataLen, uint8_t* *ppunCompressData, uint32_t* punCompressDataLen)
{
    uint8_t* pHash;
    uint8_t* punCompre;
    uint32_t CompreLen, unCompreLen;
    BYTE Node[2][256], Child[256];

    *ppunCompressData   = 0;
    *punCompressDataLen = 0;


    if (((COMPRESSHEADER*)HashData)->Magic != 0xFF435031)
    {
        AppendMsg(L"Hash数据不匹配\r\n");
        return -3;
    }

    unCompreLen = ((COMPRESSHEADER*)HashData)->unCompressLen;
    if (NULL == (punCompre = (uint8_t*)VirtualAlloc(NULL, unCompreLen, MEM_COMMIT, PAGE_READWRITE)))
    {
        AppendMsg(L"内存不足\r\n");
        return -2;
    }

    CompreLen = HashDataLen - sizeof(COMPRESSHEADER);
    pHash     = (uint8_t*)HashData + sizeof(COMPRESSHEADER);


    uint32_t curHash = 0, curUncom = 0, cnt;
    while (curHash < CompreLen)
    {
        for (uint32_t i=0; i<256; ++i)
            Node[0][i] = (BYTE)i;

        for (uint32_t i=0; i<256;)
        {
            cnt = pHash[curHash++];

            if (cnt > 127)
            {
                i += cnt - 127;
                cnt = 0;
            }

            if (i > 255)
                break;

            ++cnt;

            for (uint32_t j=0; j<cnt; ++j, ++i)
            {
                Node[0][i] = pHash[curHash++];

                if (Node[0][i] != i)
                    Node[1][i] = pHash[curHash++];
            }
        }

        if (((COMPRESSHEADER*)HashData)->Flag & 1)
        {
            cnt = *(PWORD)(pHash + curHash);
            curHash += 2;
        }
        else
        {
            cnt = *(uint32_t*)(pHash + curHash);
            curHash += 4;
        }

        uint32_t j = 0, idx;
        while (1)
        {
            if (j > 0)
                idx = Child[--j];
            else
            {
                if (!cnt)
                    break;

                --cnt;
                idx = pHash[curHash++];
            }

            if (Node[0][idx] == idx)
                punCompre[curUncom++] = (BYTE)idx;
            else
            {///////////////////////////////////////////////////////////
                Child[j++] = Node[1][idx];
                Child[j++] = Node[0][idx];
            }///////////////////////////////////////////////////////////
        }
    }

    if (curUncom != ((COMPRESSHEADER*)HashData)->unCompressLen)
    {
        VirtualFree(punCompre, 0, MEM_RELEASE);
        AppendMsg(L"Hash数据解压错误\r\n");
        return -4;
    }

    *ppunCompressData   = punCompre;
    *punCompressDataLen = curUncom;
    return 0;
}


int Mangekyoo_1_2_3::GetPackageDirectory(const HANDLE hPack, const PACKHEADER *ph, vector<PACKIDX>& Idx)
{
    uint32_t	Key, HashSize, RawIndexLen = 0;
    int		    ret = 0;
    uint8_t*	HashData;
    uint8_t*    RawIndex = 0;
    BYTE	    Buf[0x440];
    DWORD       R;


    SetFilePointer(hPack, -(int)sizeof(Buf), 0, FILE_END);
    ReadFile(hPack, Buf, sizeof(Buf), &R, 0);

    Key = 0xfffffff & GenerateKey(Buf + 0x24, 256);
    Decode(Buf, 32, Key);


    HashSize   = *(uint32_t*)(Buf + 0x20);
    HashData = new BYTE[HashSize];
    if (!HashData)
    {
        AppendMsg(L"内存不足\r\n");
        return -2;
    }

    SetFilePointer(hPack, -(int)(sizeof(Buf) + HashSize), 0, FILE_END);
    ReadFile(hPack, HashData, HashSize, &R, 0);



    if (!strcmp((char*)HashData, "HashVer1.4"))
    {
        Decode(HashData + sizeof(HASHHEADER14), ((HASHHEADER13*)HashData)->DataSize, 0x428);
        ret = Decompress(HashData + sizeof(HASHHEADER14), ((HASHHEADER13*)HashData)->DataSize,
                &RawIndex, &RawIndexLen);
    }
    else if (!strcmp((char*)HashData, "HashVer1.3"))
    {
        Decode(HashData + sizeof(HASHHEADER13), ((HASHHEADER13*)HashData)->DataSize, 0x428);
        ret = Decompress(HashData + sizeof(HASHHEADER13), ((HASHHEADER13*)HashData)->DataSize,
                &RawIndex, &RawIndexLen);
    }
    else
    {
        AppendMsg(L"Hash数据不匹配\r\n");
        return -3;
    }



    if (ret)
    {
        delete[] HashData;
        return ret;
    }

    uint32_t OffsetHigh = (uint32_t)(ph->IndexOffset >> 32);
    SetFilePointer(hPack, (ULONG)ph->IndexOffset, (PLONG)&OffsetHigh, FILE_BEGIN);
    for (uint32_t i=0; i<ph->FileNum; ++i)
    {
        WORD NameLen, j;
        BYTE NameKey;
        PACKIDX pi;

        ReadFile(hPack, &NameLen, 2, &R, 0);
        vector<char> tmp(NameLen);
        ReadFile(hPack, tmp.data(), tmp.size(), &R, 0);
        pi.Name.assign(tmp.begin(), tmp.end());

        NameKey = (BYTE)(NameLen + (Key ^ 0x3e));
        for (j=0; j<NameLen; ++j)
            pi.Name[j] ^= ((j+1) ^ NameKey) + (j+1);

        ReadFile(hPack, &pi.Offset,        8, &R, 0);
        ReadFile(hPack, &pi.CompressLen,   4, &R, 0);
        ReadFile(hPack, &pi.unCompressLen, 4, &R, 0);
        ReadFile(hPack, &pi.IsCompressed,  4, &R, 0);
        ReadFile(hPack, &pi.IsEncrypted,   4, &R, 0);
        ReadFile(hPack, &pi.Hash,          4, &R, 0);

        pi.Key = Key;

        Idx.push_back(pi);
    }

    if (Idx.size() != ph->FileNum)
    {
        AppendMsg(L"封包索引提取失败\r\n");
        return -4;
    }
    return 0;
}


int Mangekyoo_1_2_3::GetKeyBuf(uint8_t* *ppExeKeyBuf, uint32_t* pdwExeKeyLen, uint8_t* *ppKeyFile, uint32_t* pdwKeyFileLen)
{
    HANDLE hExe, hKey;
    DWORD  R;
    int    ret = 0;
    uint8_t*  retKey = 0;
    uint8_t*  retExe = 0;
    uint8_t*  pExeBuf = 0;
    uint8_t*  p = nullptr;


    hExe = CreateFile(g_ExePath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    hKey = CreateFile(g_KeyPath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (hExe == INVALID_HANDLE_VALUE || hKey == INVALID_HANDLE_VALUE)
    {
        AppendMsg(L"exe或KeyFile打开失败\r\n");
        return -1;
    }


    // 处理exe
    uint32_t dwExeLen = GetFileSize(hExe, 0);
    pExeBuf = (uint8_t*)VirtualAlloc(NULL, dwExeLen, MEM_COMMIT, PAGE_READWRITE);
    if (!pExeBuf)
    {
        AppendMsg(L"内存分配失败\r\n");
        ret = -2;
        goto RELEASE;
    }

    ReadFile(hExe, pExeBuf, dwExeLen, &R, 0);

    for (p = pExeBuf+dwExeLen-6; p>=pExeBuf; --p)
        if (!memcmp(p, "\x05TIcon", 6))
            break;
    if (p < pExeBuf)
    {
        AppendMsg(L"无法在指定exe中找到Key信息\r\n");
        ret = -7;
        goto RELEASE;
    }

    retExe = (uint8_t*)VirtualAlloc(NULL, 256, MEM_COMMIT, PAGE_READWRITE);
    if (!pExeBuf)
    {
        AppendMsg(L"内存分配失败\r\n");
        ret = -2;
        goto RELEASE;
    }

    memcpy(retExe, p+6, 256);


    // 处理KeyFile
    uint32_t dwKeyFileLen = GetFileSize(hKey, 0);
    retKey = (uint8_t*) VirtualAlloc(NULL, dwKeyFileLen, MEM_COMMIT, PAGE_READWRITE);
    if (!pExeBuf)
    {
        AppendMsg(L"内存分配失败\r\n");
        ret = -2;
        goto RELEASE;
    }

    ReadFile(hKey, retKey, dwKeyFileLen, &R, 0);

    *ppExeKeyBuf   = retExe;
    *ppKeyFile     = retKey;
    *pdwExeKeyLen  = 256;
    *pdwKeyFileLen = dwKeyFileLen;

RELEASE:
    CloseHandle(hExe);
    CloseHandle(hKey);

    if (pExeBuf) VirtualFree(pExeBuf, 0, MEM_RELEASE);

    if (ret)
    {
        if (retExe) VirtualFree(retExe, 0, MEM_RELEASE);
        if (retKey) VirtualFree(retKey, 0, MEM_RELEASE);
        *ppExeKeyBuf = 0;
        *ppKeyFile   = 0;
        *pdwExeKeyLen = *pdwKeyFileLen = 0;
    }

    return ret;
}


int Mangekyoo_1_2_3::HandleDPNG(uint8_t* Data, uint32_t Len, const wchar_t *CurDir, const wchar_t *UniName)
{
    if (Len < 4 || *(uint32_t*)Data != 0x474e5044) // "DPNG"
        return 0;

    wchar_t Format[MAX_PATH], NewName[MAX_PATH];
    int     ret = 0;

    StringCchCopy(Format, MAX_PATH, UniName);
    int k = wcslen(Format);
    while (k>0 && Format[k-1] != '.') --k;
    Format[k-1] = 0;
    StringCchCat(Format, MAX_PATH, L"+%02d+x%dy%d.png");


    DPNGHEADER *pdh = (DPNGHEADER*) Data;
    Data += sizeof(DPNGHEADER);

    for (uint32_t i = 0; i < pdh->IdxNum; ++i)
    {
        DPNGIDX* idx = (DPNGIDX*) Data;
        Data += sizeof(DPNGIDX);

        if (idx->Length)
        {
            StringCchPrintf(NewName, MAX_PATH, Format, i, idx->Offsetx, idx->Offsety);
            if (!SplitFileNameAndSave(CurDir, NewName, Data, idx->Length))
                ++ret;
        }

        Data += idx->Length;
    }

    return ret;
}


int Mangekyoo_1_2_3::ExtractResource(const HANDLE hPack, const vector<PACKIDX>& Idx, const wchar_t *CurDir, const wchar_t *PackName)
{
    uint32_t dwunCompreLen, dwExeKeyLen, dwKeyFileLen;
    uint32_t dwFileProcesses = 0;
    uint8_t* unCompre;
    uint8_t* pExeKeyBuf;
    uint8_t* pKeyFile;
    int      ret = 0;
    DWORD    R;
    wchar_t	 UniName[MAX_PATH], MagBuf[MAX_PATH];


    ret = GetKeyBuf(&pExeKeyBuf, &dwExeKeyLen, &pKeyFile, &dwKeyFileLen);
    if (ret)
    {
        AppendMsg(L"Key获取失败\r\n");
        return ret;
    }


    for (uint32_t i=0; i<Idx.size(); ++i)
    {
        uint8_t* PackData = (uint8_t*)VirtualAlloc(NULL, Idx[i].CompressLen * 2, MEM_COMMIT, PAGE_READWRITE);
        if (!PackData)
        {
            AppendMsg(L"内存不足\r\n");
            return -2;
        }


        MultiByteToWideChar(932, 0, Idx[i].Name.c_str(), MAX_PATH, UniName, MAX_PATH);


        int err = 0;
        LONG OffsetHigh = (LONG)(Idx[i].Offset >> 32);

        err = SetFilePointer(hPack, (uint32_t)Idx[i].Offset, &OffsetHigh, FILE_BEGIN);
        if (err == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
        {
            StringCchPrintf(MagBuf, MAX_PATH, L"设置文件指针失败, 跳过 %s\r\n", UniName);
            AppendMsg(MagBuf);
            goto CONTINUE;
        }

        err = ReadFile(hPack, PackData, Idx[i].CompressLen, &R, 0);
        if (!err || R != Idx[i].CompressLen)
        {
            StringCchPrintf(MagBuf, MAX_PATH, L"读取文件出错, 跳过 %s\r\n", UniName);
            AppendMsg(MagBuf);
            goto CONTINUE;
        }


        if (GenerateKey(PackData, Idx[i].CompressLen) != Idx[i].Hash)
        {
            StringCchPrintf(MagBuf, MAX_PATH, L"文件Hash不匹配, 跳过 %s\r\n", UniName);
            AppendMsg(MagBuf);
            goto CONTINUE;
        }

        if (Idx[i].IsEncrypted)
            ret = DecodeFileData(PackData, Idx[i], pExeKeyBuf, dwExeKeyLen, pKeyFile, dwKeyFileLen);

        if (Idx[i].IsCompressed)
        {
            Decompress(PackData, Idx[i].CompressLen, &unCompre, &dwunCompreLen);
            if (ret)
                goto CONTINUE;

            VirtualFree(PackData, 0, MEM_RELEASE);
            PackData = 0;
        }
        else
        {
            unCompre		= PackData;
            dwunCompreLen	= Idx[i].CompressLen;
        }


        if (HandleDPNG(unCompre, dwunCompreLen, CurDir, UniName))
        {
            ++dwFileProcesses;
            goto CONTINUE;
        }


        if (!SplitFileNameAndSave(CurDir, UniName, unCompre, dwunCompreLen))
            ++dwFileProcesses;

CONTINUE:
        if (PackData) VirtualFree(PackData, 0, MEM_RELEASE);
        VirtualFree(unCompre, 0, MEM_RELEASE);
    }



    return dwFileProcesses;
}


int Mangekyoo_1_2_3::Entrance(const wchar_t *CurDir, const wchar_t *PackName)
{
    HANDLE			hPack;
    uint32_t		dwFileProcesses;
    DWORD           R;
    int				ret;
    PACKHEADER		ph;
    vector<PACKIDX>	Idx;
    wchar_t			MsgBuf[MAX_PATH];


    if (INVALID_HANDLE_VALUE ==
            (hPack = CreateFile(PackName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)))
    {
        AppendMsg(L"无法打开文件\r\n");
        return -1;
    }

    SetFilePointer(hPack, -(int)sizeof(PACKHEADER), 0, FILE_END);
    ReadFile(hPack, &ph, sizeof(ph), &R, 0);
    if (strcmp(ph.Maigc, "FilePackVer3.0"))
    {
        AppendMsg(L"封包类型不匹配\r\n");
        return -3;
    }

    // 提取索引目录
    if (ret = GetPackageDirectory(hPack, &ph, Idx))
        return ret;


    // 提取资源
    dwFileProcesses = ExtractResource(hPack, Idx, CurDir, PackName);

    if (dwFileProcesses == ph.FileNum)
    {
        StringCchPrintf(MsgBuf, MAX_PATH, L"[提取完成(%d)] %s\r\n", ph.FileNum, PackName);
        AppendMsg(MsgBuf);
    }
    else
    {
        StringCchPrintf(MsgBuf, MAX_PATH, L"[提取完成(%d/%d)] %s\r\n有%d个文件提取失败",
            dwFileProcesses, ph.FileNum, PackName, ph.FileNum - dwFileProcesses);
        AppendMsg(MsgBuf);
        MessageBox(0, MsgBuf, L"结果", MB_ICONWARNING);
    }
    return 0;
}





/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////





std::string ToString(const std::wstring & str)
{
    int sz = WideCharToMultiByte(932, 0, str.c_str(), -1, 0, 0, 0, 0);
    std::vector<char> vec(sz + 1);
    WideCharToMultiByte(932, 0, str.c_str(), -1, vec.data(), vec.size(), 0, 0);
    return std::string(vec.data());
}

std::wstring ToString(const std::string & str)
{
    int sz = MultiByteToWideChar(932, 0, str.c_str(), -1, 0, 0);
    std::vector<wchar_t> vec(sz + 1);
    MultiByteToWideChar(932, 0, str.c_str(), -1, vec.data(), vec.size());
    return std::wstring(vec.data());
}

DWORD ROL(DWORD x, int n)
{
    return (x << n) | (x >> (32 - n));
}

DWORD Mangekyoo_4_5::GenerateKey3_1(PVOID Buf, DWORD Len)
{
    PWORD wBuf = (PWORD)Buf;
    WORD mm0[4], mm2[4], mm3[4];

    for (DWORD i = 0; i < 4; ++i)
    {
        mm0[i] = mm2[i] = 0;
    }
    *(PDWORD)&mm3[0] = *(PDWORD)&mm3[2] = 0xa35793a7;

    for (DWORD k = 0; k < Len / 8; ++k)
    {
        for (DWORD i = 0; i < 4; ++i)
        {
            mm2[i] += mm3[i];
            mm0[i] += wBuf[i] ^ mm2[i];
        }
        *(PDWORD)&mm0[0] = ROL(*(PDWORD)&mm0[0], 3);
        *(PDWORD)&mm0[2] = ROL(*(PDWORD)&mm0[2], 3);
        wBuf += 4;
    }
    DWORD ret = ((short)mm0[0] * (short)mm0[2]) + ((short)mm0[1] * (short)mm0[3]);
    return ret;
}

void Mangekyoo_4_5::Decode(PVOID Buf, DWORD Len, DWORD Key)
{
    PDWORD dBuf = (PDWORD)Buf;
    DWORD  mm5[2], mm6[2], mm7[2];

    for (DWORD i = 0; i < 2; ++i)
    {
        mm6[i] = 0xce24f523;
        mm7[i] = 0xa73c5f9d;
        mm5[i] = (Key + Len) ^ 0xfec9753e;
    }

    for (DWORD i = 0; i < Len / 8; ++i)
    {
        for (DWORD j = 0; j < 2; ++j)
        {
            mm7[j] = (mm7[j] + mm6[j]) ^ mm5[j];
            dBuf[j] ^= mm7[j];
            mm5[j] = dBuf[j];
        }
        dBuf += 2;
    }
    return;
}

int Mangekyoo_4_5::Decode_Sub1(DWORD dwSeed, PDWORD arrDecode, PDWORD pSubFlag)
{
    // 类似于初始化
    const DWORD c = 0x8a77f473;
    unsigned long long mul = (long long)(dwSeed ^ c) * c;
    arrDecode[0] = (DWORD)mul + (DWORD)(mul >> 32);

    for (DWORD i = 0; i < 63; ++i)
    {
        long long mul = (long long)(arrDecode[i] ^ c) * c;
        arrDecode[i + 1] = (DWORD)mul + (DWORD)(mul >> 32);
    }

    *pSubFlag = 1;

    return 0;
}

int Mangekyoo_4_5::DecodeFileData(PVOID PackData, const PACKIDX& Idx, const void* secretTable)
{
    DWORD dwSum = 0x86f7e2, dwSeed = 0x4437f1, dwFileNameLen;
    DWORD SubFlag;
    DWORD arrDecode[256];

    dwFileNameLen = Idx.Wname.size();
    for (DWORD i = 0; i < dwFileNameLen; ++i)
    {
        dwSum += (WORD)Idx.Wname[i] << ((BYTE)i & 7);		// Name要无符号
        dwSeed ^= dwSum;
    }

    dwSeed += ((Idx.CompressLen ^ 0x56e213 ^ dwSum) + dwSum + Idx.CompressLen + 13 * (Idx.CompressLen & 0xffffff)) ^ Idx.Key;
    dwSeed = (dwSeed & 0xffffff) * 13;

    Decode_Sub1(dwSeed, arrDecode, &SubFlag);

    typedef union {
        ULONGLONG	Qword;
        DWORD		Dword[2];
        WORD		Word[4];
        BYTE		Byte[8];
    } UQWORD;
    UQWORD mm7, mm6;

    ULONGLONG *p = (ULONGLONG*)PackData;
    ULONGLONG *pEnd = (ULONGLONG*)PackData + Idx.CompressLen / 8;
    ULONGLONG *parr1 = (ULONGLONG*)arrDecode;

    DWORD arr1Idx = (arrDecode[8] & 0xd) * 8;

    mm7.Qword = *(ULONGLONG*)&arrDecode[6];
    while (p < pEnd)
    {
        // pxor mm7, mm6
        mm6.Qword = *(parr1 + (arr1Idx & 0xf)) ^ *((ULONGLONG*)secretTable + (arr1Idx & 0x7f));
        mm7.Qword ^= mm6.Qword;

        // paddd mm7, mm6
        mm7.Dword[0] += mm6.Dword[0];
        mm7.Dword[1] += mm6.Dword[1];

        // movq mm0, qword ptr[edi]
        // pxor mm0, mm7
        // movq mm1, mm0
        // mov1 qword ptr [edi], mm0
        *p ^= mm7.Qword;

        // paddb mm7, mm1   (*ppd)
        for (DWORD i = 0; i < 8; ++i)
            mm7.Byte[i] += *((PBYTE)p + i);

        // pxor mm7, mm1
        mm7.Qword ^= *p;

        // pslld mm7, 1
        mm7.Dword[0] <<= 1;
        mm7.Dword[1] <<= 1;

        // paddw mm7, mm1
        for (DWORD i = 0; i < 4; ++i)
            mm7.Word[i] += *((PWORD)p + i);

        ++p;
        arr1Idx = ++arr1Idx & 0x7f;
    }
    return 0;
}

int Mangekyoo_4_5::GetPackageDirectory(const HANDLE hPack, const PACKHEADER *ph, vector<PACKIDX>& Idx)
{
    DWORD	Key, HashSize, R;
    int		ret = 0;
    PBYTE	RawIndex = 0;
    BYTE	Buf[0x440];

    SetFilePointer(hPack, -(int)sizeof(Buf), 0, FILE_END);
    ReadFile(hPack, Buf, sizeof(Buf), &R, 0);

    if (ph->Maigc == PackVer3_1)
        Key = 0xfffffff & GenerateKey3_1(Buf + 0x24, 256);
    else
    {
        AppendMsg(L"GetPackageDirectory 不支持的 PackVer\r\n");
        return -2;
    }

    Decode(Buf, 32, Key);


    HashSize = *(PDWORD)(Buf + 0x20);
    vector<uint8_t> HashData(HashSize);
    SetFilePointer(hPack, -(int)(sizeof(Buf) + HashSize), 0, FILE_END);
    ReadFile(hPack, HashData.data(), HashSize, &R, 0);



    if ((char*)HashData.data() == HashVer1_4)
    {
        uint32_t RawIndexLen = 0;
        Decode(HashData.data() + sizeof(HASHHEADER14), ((HASHHEADER13*)HashData.data())->DataSize, 0x428);
        ret = Mangekyoo_1_2_3::Decompress(HashData.data() + sizeof(HASHHEADER14), ((HASHHEADER13*)HashData.data())->DataSize,
            &RawIndex, &RawIndexLen);
    }
    else
    {
        AppendMsg(L"Hash数据不匹配\r\n");
        return -3;
    }

    if (ret)
        return ret;

    if (ph->Maigc == PackVer3_1)
    {
        DWORD OffsetHigh = (DWORD)(ph->IndexOffset >> 32);
        SetFilePointer(hPack, (ULONG)ph->IndexOffset, (PLONG)&OffsetHigh, FILE_BEGIN);
        DWORD tmpKey = (Key >> 16) ^ ((WORD)Key);
        for (DWORD i = 0; i < ph->FileNum; ++i)
        {
            WORD NameLen, j;
            PACKIDX pi;

            ReadFile(hPack, &NameLen, 2, &R, 0);
            std::vector<wchar_t> Name(NameLen * 2);
            ReadFile(hPack, Name.data(), Name.size(), &R, 0);

            const WORD NameKey = static_cast<WORD>(NameLen * NameLen) ^ (NameLen ^ 0x3e13 ^ tmpKey);
            WORD tmpNameKey = NameKey;
            for (j = 0; j < NameLen; ++j)
            {
                tmpNameKey = tmpNameKey * 8 + j + NameKey;
                Name[j] ^= tmpNameKey;
            }
            pi.Wname.assign(Name.begin(), Name.begin() + NameLen);

            ReadFile(hPack, &pi.Offset, 8, &R, 0);
            ReadFile(hPack, &pi.CompressLen, 4, &R, 0);
            ReadFile(hPack, &pi.unCompressLen, 4, &R, 0);
            ReadFile(hPack, &pi.IsCompressed, 4, &R, 0);
            ReadFile(hPack, &pi.IsEncrypted, 4, &R, 0);
            ReadFile(hPack, &pi.Hash, 4, &R, 0);

            pi.Key = Key;
            pi.HashVer = (char*)HashData.data();

            Idx.push_back(pi);
        }
    }


    if (Idx.size() != ph->FileNum)
    {
        AppendMsg(L"封包索引提取失败\r\n");
        return -4;
    }
    return 0;
}

int Mangekyoo_4_5::GetKeyBuf(std::vector<char>& exeKeyBuf)
{
    HANDLE hExe;
    DWORD  R;
    int    ret = 0;
    PBYTE  retKey = 0, retExe = 0, p;
    std::vector<uint8_t> pExeBuf;

    hExe = CreateFile(g_ExePath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hExe == INVALID_HANDLE_VALUE)
    {
        AppendMsg(L"exe打开失败\r\n");
        return -1;
    }

    // 处理exe
    DWORD dwExeLen = GetFileSize(hExe, 0);
    pExeBuf.resize(dwExeLen);
    ReadFile(hExe, pExeBuf.data(), pExeBuf.size(), &R, 0);

    const string flag = "KeyFile ver";
    for (p = pExeBuf.data() + pExeBuf.size() - flag.size(); p >= pExeBuf.data(); --p)
        if (!memcmp(p, flag.c_str(), flag.size()))
            break;
    if (p < pExeBuf.data())
    {
        AppendMsg(L"无法在指定exe中找到Key信息\r\n");
        ret = -7;
        goto RELEASE;
    }

    exeKeyBuf.assign(p, p + 256);

RELEASE:
    CloseHandle(hExe);
    return ret;
}

int Mangekyoo_4_5::DecodeKeyFileData(PVOID PackData, const PACKIDX& Idx)
{
    assert(Idx.IsEncrypted == 1);

    DWORD dwSum = 0x85f532;
    DWORD dwSeed = 0x33f641;

    const uint32_t dwFileNameLen = Idx.Wname.size();
    for (DWORD i = 0; i < dwFileNameLen; ++i)
    {
        dwSum += ((uint32_t)Idx.Wname[i] << (i & 7));
        dwSeed ^= dwSum;
    }
    dwSeed += ((Idx.CompressLen ^ 0x8f32dc ^ dwSum) + dwSum + Idx.CompressLen + 7 * (Idx.CompressLen & 0xffffff)) ^ Idx.Key;
    dwSeed = (dwSeed & 0xffffff) * 9;

    // 初始化
    DWORD arrDecode[256] = { 0 };
    uint32_t val = dwSeed;
    for (int i = 0; i < 0x40; ++i)
    {
        constexpr uint32_t magic = 0x8df21431;

        val ^= magic;
        uint64_t mul = (uint64_t)val * magic;
        val = arrDecode[i] = (mul & 0xffffffff) + (mul >> 32);
    }

    typedef union {
        ULONGLONG	Qword;
        DWORD		Dword[2];
        WORD		Word[4];
        BYTE		Byte[8];
    } UQWORD;
    UQWORD Tmp;
    Tmp.Dword[1] = arrDecode[7];
    Tmp.Dword[0] = arrDecode[6];

    ULONGLONG *p = (ULONGLONG*)PackData;
    ULONGLONG *pEnd = (ULONGLONG*)PackData + Idx.CompressLen / 8;
    uint8_t* parr1 = (uint8_t*)arrDecode;

    DWORD arr1Idx = (arrDecode[0xd] & 0xf) * 8;
    while (p < pEnd)
    {
        // pxor mm7, mm6
        Tmp.Qword ^= *(uint64_t*)(parr1 + arr1Idx);

        // paddd mm7, mm6
        Tmp.Dword[0] += *(uint32_t*)(parr1 + arr1Idx);
        Tmp.Dword[1] += *(uint32_t*)(parr1 + arr1Idx + 4);

        // movq mm0, qword ptr[edi]
        // pxor mm0, mm7
        // movq mm1, mm0
        // mov1 qword ptr [edi], mm0
        *p ^= Tmp.Qword;

        // paddb mm7, mm1   (*ppd)
        for (DWORD i = 0; i < 8; ++i)
            Tmp.Byte[i] += *((PBYTE)p + i);

        // pxor mm7, mm1
        Tmp.Qword ^= *p;

        // pslld mm7, 1
        Tmp.Dword[0] <<= 1;
        Tmp.Dword[1] <<= 1;

        // paddw mm7, mm1
        for (DWORD i = 0; i < 4; ++i)
            Tmp.Word[i] += *((PWORD)p + i);

        ++p;
        arr1Idx = (arr1Idx + 8) & 0x7f;
    }
    return 0;
}

int Mangekyoo_4_5::HandleDPNG(PBYTE Data, DWORD Len, const wchar_t *CurDir, const wchar_t *UniName)
{
    if (Len < 4 || *(PDWORD)Data != 0x474e5044) // "DPNG"
        return 0;

    wchar_t Format[MAX_PATH], NewName[MAX_PATH];
    int     ret = 0;

    wcscpy_s(Format, MAX_PATH, UniName);
    int k = wcslen(Format);
    while (k > 0 && Format[k - 1] != '.') --k;
    Format[k - 1] = 0;
    wcscat_s(Format, MAX_PATH, L"+%02d+x%dy%d.png");


    DPNGHEADER *pdh = (DPNGHEADER*)Data;
    Data += sizeof(DPNGHEADER);

    for (DWORD i = 0; i < pdh->IdxNum; ++i)
    {
        DPNGIDX* idx = (DPNGIDX*)Data;
        Data += sizeof(DPNGIDX);

        if (idx->Length)
        {
            swprintf_s(NewName, MAX_PATH, Format, i, idx->Offsetx, idx->Offsety);
            if (!Mangekyoo_1_2_3::SplitFileNameAndSave(CurDir, NewName, Data, idx->Length))
                ++ret;
        }

        Data += idx->Length;
    }

    return ret;
}

int Mangekyoo_4_5::ExtractResource(const PACKHEADER& ph, const HANDLE hPack, const vector<PACKIDX>& Idx, const wchar_t *CurDir, const wchar_t *PackName)
{
    DWORD R;
    DWORD dwFileProcesses = 0;
    PBYTE unCompre = 0, pExeKeyBuf = 0;
    int   ret = 0;
    std::wstring UniName;
    vector<char> pKeyFile;

    ret = GetKeyBuf(pKeyFile);
    if (ret)
    {
        AppendMsg(L"Key获取失败\r\n");
        return ret;
    }

    bool is_mangekyou4 = string((char*)(pKeyFile.data() + 0x20)).find("BisyoujyoMangekyou_TumiToBatu") != string::npos;
    bool is_mangekyou5 = string((char*)(pKeyFile.data() + 0x20)).find("bisyoujyomangekyou_kotowari") != string::npos;
    bool is_annabelmaidgarden = string((char*)(pKeyFile.data() + 0x20)).find("AnnabelMaidGarden 2020 DL") != string::npos;


    for (DWORD i = 0; i < Idx.size(); ++i)
    {
        PBYTE PackData = (PBYTE)VirtualAlloc(NULL, Idx[i].CompressLen * 2, MEM_COMMIT, PAGE_READWRITE);
        if (!PackData)
        {
            AppendMsg(L"内存不足\r\n");
            return -2;
        }

        if (!Idx[i].Wname.empty())
            UniName = Idx[i].Wname;
        else
            UniName = ToString(Idx[i].Name);


        int err = 0;
        LONG OffsetHigh = (LONG)(Idx[i].Offset >> 32);

        err = SetFilePointer(hPack, (DWORD)Idx[i].Offset, &OffsetHigh, FILE_BEGIN);
        if (err == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
        {
            wstringstream wss;
            wss << L"设置文件指针失败, 跳过 " << UniName << L"\r\n";
            AppendMsg(wss.str().c_str());
            goto CONTINUE;
        }

        err = ReadFile(hPack, PackData, Idx[i].CompressLen, &R, 0);
        if (!err || R != Idx[i].CompressLen)
        {
            wstringstream wss;
            wss << L"读取文件出错, 跳过 " << UniName << L"\r\n";
            AppendMsg(wss.str().c_str());
            goto CONTINUE;
        }

        if (ph.Maigc == PackVer3_1)
        {
            if (GenerateKey3_1(PackData, Idx[i].CompressLen) != Idx[i].Hash)
            {
                wstringstream wss;
                wss << L"文件Hash不匹配, 跳过 " << UniName << L"\r\n";
                AppendMsg(wss.str().c_str());
                goto CONTINUE;
            }
        }
        else
        {
            AppendMsg(L"不支持的封包版本\r\n");
            goto CONTINUE;
        }

        if (Idx[i].HashVer == HashVer1_4)
        {
            if (Idx[i].IsEncrypted == 1)
                ret = DecodeKeyFileData(PackData, Idx[i]);
            else if (Idx[i].IsEncrypted >= 2)
            {
                const void * extra = nullptr;
                if (is_mangekyou5)
                    extra = Mangekyoo5;
                else if (is_mangekyou4)
                    extra = Mangekyoo4;
                else if (is_annabelmaidgarden)
                    extra = extra = AnnabelMaidGarden.data();
                else
                {
                    AppendMsg(L"找不到对应解密表，暂不支持的游戏\r\n");
                    goto CONTINUE;
                }
                ret = DecodeFileData(PackData, Idx[i], extra);
            }
        }
        else
        {
            AppendMsg(L"不支持的 Hash 版本\r\n");
        }

        uint32_t dwunCompreLen = 0;
        if (Idx[i].IsCompressed)
        {
            Mangekyoo_1_2_3::Decompress(PackData, Idx[i].CompressLen, &unCompre, &dwunCompreLen);
            if (ret)
                goto CONTINUE;

            VirtualFree(PackData, 0, MEM_RELEASE);
            PackData = 0;
        }
        else
        {
            unCompre = PackData;
            dwunCompreLen = Idx[i].CompressLen;
        }


        if (HandleDPNG(unCompre, dwunCompreLen, CurDir, UniName.c_str()))
        {
            ++dwFileProcesses;
            goto CONTINUE;
        }


        if (!Mangekyoo_1_2_3::SplitFileNameAndSave(CurDir, UniName.c_str(), unCompre, dwunCompreLen))
            ++dwFileProcesses;

    CONTINUE:
        if (PackData) VirtualFree(PackData, 0, MEM_RELEASE);
        VirtualFree(unCompre, 0, MEM_RELEASE);
    }

    return dwFileProcesses;
}

int Mangekyoo_4_5::Entrance(const wchar_t *CurDir, const wchar_t *PackName)
{
    HANDLE			hPack;
    DWORD			dwFileProcesses, R;
    int				ret;
    PACKHEADER		ph;
    vector<PACKIDX>	Idx;

    if (INVALID_HANDLE_VALUE ==
        (hPack = CreateFile(PackName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)))
    {
        AppendMsg(L"无法打开文件\r\n");
        return -1;
    }

    SetFilePointer(hPack, -(int)sizeof(PACKHEADER), 0, FILE_END);
    ReadFile(hPack, &ph, sizeof(ph), &R, 0);
    if (ph.Maigc != PackVer3_0 && ph.Maigc != PackVer3_1)
    {
        AppendMsg(L"封包类型不支持\r\n");
        return -3;
    }

    // 提取索引目录
    if (ret = GetPackageDirectory(hPack, &ph, Idx))
        return ret;


    // 提取资源
    dwFileProcesses = ExtractResource(ph, hPack, Idx, CurDir, PackName);

    if (dwFileProcesses == ph.FileNum)
    {
        wstringstream ss;
        ss << L"[提取完成(" << ph.FileNum << L")] " << PackName << L"\r\n";
        AppendMsg(ss.str().c_str());
    }
    else
    {
        wstringstream wss;
        wss << L"[提取完成(" << dwFileProcesses << "/" << ph.FileNum << L")] " << PackName << L"\r\n有" << ph.FileNum - dwFileProcesses << L"个文件提取失败";
        AppendMsg(wss.str().c_str());
        MessageBox(0, wss.str().c_str(), L"结果", MB_ICONWARNING);
    }
    return 0;
}