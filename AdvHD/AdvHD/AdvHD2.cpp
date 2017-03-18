
#include "AdvHD2.h"

#include <assert.h>
#include <Windows.h>
#include <strsafe.h>
#include "functions.h"
#include "AdvHD2.h"

extern HWND g_hEdit;

extern bool IsPnaFile(const unsigned char *Data);
extern int ExtractPNAPFile(unsigned char *Data, const wchar_t *CurDir, const wchar_t *FileName);

    
int AdvHD2::Entrance(const wchar_t *CurDir, const wchar_t *PackName)
{
	int ret = 0;
	DWORD BytesRead;
	HANDLE hPack;
	struct ARC_HEADER packHeader;
	wchar_t MsgBuf[MAX_PATH], FileName[MAX_PATH];
	
	hPack = CreateFile(PackName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if (hPack == INVALID_HANDLE_VALUE)
	{
		StringCchPrintf(MsgBuf, MAX_PATH, L"无法打开文件%s\r\n", PackName);
		AppendMsg(g_hEdit, MsgBuf);
		return -1;
	}

	DWORD ExtractFileCount = 0;
    DWORD totalFileCount = 0;

    ReadFile(hPack, &packHeader.TypeNum, 4, &BytesRead, NULL);
    packHeader.TypeList = new ARC_HEADER_TYPE[packHeader.TypeNum + 1];       // 多设置一个
    memset(packHeader.TypeList, 0, sizeof(ARC_HEADER_TYPE) * (packHeader.TypeNum + 1));
    for (int i=0; i<packHeader.TypeNum; ++i)
    {
        assert(sizeof(ARC_HEADER_TYPE) == 12);
	    ReadFile(hPack, &packHeader.TypeList[i], sizeof(ARC_HEADER_TYPE), &BytesRead, NULL);
        assert(BytesRead == sizeof(ARC_HEADER_TYPE));
        totalFileCount += packHeader.TypeList[i].FileCount;
    }
    // 填充最后一项
    packHeader.TypeList[packHeader.TypeNum].StartIdxOffset = -1;


    for (int iType = 0; iType < packHeader.TypeNum; ++iType)
    {
        for (int iCount = 0; iCount < packHeader.TypeList[iType].FileCount; ++iCount)
        {
            BOOL bResult = SetFilePointer(hPack, packHeader.TypeList[iType].StartIdxOffset + iCount * sizeof(IDX_ENTRY), NULL, FILE_BEGIN);
            assert(bResult);
            assert(packHeader.TypeList[iType].StartIdxOffset + iCount * sizeof(IDX_ENTRY)
                < packHeader.TypeList[iType + 1].StartIdxOffset);

            IDX_ENTRY stIdx;
            ReadFile(hPack, &stIdx, sizeof(IDX_ENTRY), &BytesRead, NULL);

            // 根据索引读取文件数据
            PVOID pData = VirtualAlloc(NULL, stIdx.Size, MEM_COMMIT, PAGE_READWRITE);
		    if (!pData)
		    {
			    AppendMsg(g_hEdit, L"内存不足！\r\n");
			    CloseHandle(hPack);
			    return -2;
		    }

		    SetFilePointer(hPack, stIdx.Offset, NULL, FILE_BEGIN);
		    ReadFile(hPack, pData, stIdx.Size, &BytesRead, NULL);

            char Buffer[64];
            StringCbPrintfA(Buffer, sizeof(Buffer), "%s.%s",
                stIdx.NameWithoutSuffix, packHeader.TypeList[iType].TypeName);
            MultiByteToWideChar(CP_ACP, 0, Buffer, -1, FileName, MAX_PATH);
		
		    if (IsPnaFile((unsigned char*)pData))
		    {
			    ExtractPNAPFile((unsigned char*)pData, CurDir, FileName);
			    ++ExtractFileCount;
		    }
		    else if (!SplitFileNameAndSave(CurDir, FileName, pData, stIdx.Size))
			    ++ExtractFileCount;

		    VirtualFree(pData, 0, MEM_RELEASE);
        }
    }

	StringCchPrintf(MsgBuf, MAX_PATH, L"共提取文件(%d/%d)个   --   %s\r\n",
			ExtractFileCount, totalFileCount, PackName);
	AppendMsg(g_hEdit, MsgBuf);
	return 0;
}
