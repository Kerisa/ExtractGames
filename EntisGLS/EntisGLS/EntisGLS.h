
#pragma once


typedef unsigned long long QWORD;


#pragma pack (1)

enum FileTypeId
{
	fidArchive			= 0x02000400,
	fidRasterizedImage	= 0x03000100,
	fidEGL3DSurface		= 0x03001100,
	fidEGL3DModel		= 0x03001200,
	fidUndefinedEMC		= -1,
};


enum FileAttribute
{
	attrNormal			= 0x00000000,
	attrReadOnly		= 0x00000001,
	attrHidden			= 0x00000002,
	attrSystem			= 0x00000004,
	attrDirectory		= 0x00000010,
	attrEndOfDirectory	= 0x00000020,
	attrNextDirectory	= 0x00000040,
	attrFileNameUTF8	= 0x01000000,
};


enum EncodeType
{
	encodeRaw			= 0x00000000,
	encodeERISA			= 0x80000010,
	encodeCrypt32		= 0x20000000,
	encodeERISACrypt32	= 0xA0000010,
	etBSHFCrypt			= 0x40000000,
	etERISACrypt		= 0xC0000010,
};


struct FILE_TIME
{
	unsigned char	nSecond;
	unsigned char	nMinute;
	unsigned char	nHour;
	unsigned char	nWeek;
	unsigned char	nDay;
	unsigned char	nMonth;
	unsigned short	nYear;
};


typedef struct 
{
	unsigned long long	nBytes;
	unsigned long		nAttribute;
	unsigned long		nEncodeType;
	unsigned long long	nOffsetPos;
	FILE_TIME			ftFileTime;
} _FILEENTRY;


typedef struct 
{
	unsigned long long	qBytes;
	unsigned long		nAttribute;
	unsigned long		nEncodeType;
	unsigned long long	qOffset;
	unsigned long		nExtraInfoLen;
	unsigned char	   *pExtraInfo;
	wchar_t				wName[350];
} FILEENTRY;


typedef struct
{
	unsigned long	nCRC32;			// CRC32
	unsigned long	nDecrypeKey[1];	// 暗号鍵
} FILEEXTRAINFO;


typedef struct {
	char				PackageMagic[16];	// "Entis\x1A\x00\x00\x00\x04\x00\x02\x00\x00\x00\x00"
	char				Format[40];
	unsigned long long	PackageLen;			// 整个封包文件除NOAHEADER以外的长度
} NOAHEADER;


typedef struct {
	char				DirEntryMagic[8];	// "DirEntry"
	unsigned long long	IndexLen;			// 全部文件索引表的长度(索引表紧随其后)
} NOADIRENTRY;


typedef struct {
	char				FileDataMagic[8];	// "filedata"
	unsigned long long	FileRecordLen;
} NOAFILEDATA;



extern void AppendMsg(const wchar_t *szBuffer);

int Entrance(const wchar_t *PackName, const wchar_t *CurDir);