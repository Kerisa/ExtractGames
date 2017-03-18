
#pragma once 


namespace AdvHD2        // type 2
{
    struct ARC_HEADER
    {
	    unsigned long TypeNum;
	    struct ARC_HEADER_TYPE *TypeList;
    };

    struct ARC_HEADER_TYPE
    {
        char TypeName[4];
        int  FileCount;
        int  StartIdxOffset;
    };
    
#pragma pack(1)
    struct IDX_ENTRY
    {
        char NameWithoutSuffix[13];
	    unsigned long Size;
	    unsigned long Offset;
    };
#pragma pack()

    int Entrance(const wchar_t *CurDir, const wchar_t *PackName);
}