
#pragma once



#pragma pack(1)


typedef struct
{
	char Maigc[16];			// "FilePackVer3.0"
	unsigned long FileNum;
	unsigned long long IndexOffset;
} PACKHEADER;


typedef struct
{
	char Magic[16];			// "HashVer1.4"
	unsigned long unKnown1;
	unsigned long FileNum;
	unsigned long unKnown2;	// FileNum * 2
	unsigned long DataSize;
//	unsigned long unKnown3;
//	char ZeroBytes[32];
} HASHHEADER;


typedef struct
{
	unsigned long Magic;	// 0xff435031
	unsigned long Flag;
	unsigned long unCompressLen;
} COMPRESSHEADER;


typedef struct
{
	unsigned long long	Offset;
	unsigned long		CompressLen;
	unsigned long		unCompressLen;
	unsigned long		IsCompressed;
	unsigned long		IsEncrypted;
	unsigned long		Hash;
	unsigned long		Key;
	char				Name[MAX_PATH];	
} PACKIDX;


typedef struct
{
	unsigned long Magic;		 // "DPNG" - 0x474e5044
	unsigned long unKnown1;
	unsigned long IdxNum;
	unsigned long Width;
	unsigned long Height;
} DPNGHEADER;


typedef struct
{
	unsigned long Offsetx;
	unsigned long Offsety;
	unsigned long Width;
	unsigned long Height;
	unsigned long Length;
	unsigned long unKnown1;
	unsigned long unKnown2;
} DPNGIDX;


#pragma pack()


extern wchar_t g_ExePath[MAX_PATH];
extern wchar_t g_KeyPath[MAX_PATH];


extern void AppendMsg(const wchar_t *MsgBuf);