#pragma once

typedef struct
{
	char Unknown[128]{ 0 };
	//int  Type{ 0 };		// ?
	//int  Unknown1{ 0 };	// 0x80
	//char Unknown2[32];
	//int  FileLegth{ 0 };
	//char Unknown3[54];	// padding 0
	int FileCount{ 0 };
} BIP_TYPE2_HEADER;


typedef struct 
{
	int Unknown1;
} BIP_TYPE2_ENTRY;


typedef struct
{
	char   Magic[8]{ 0 };		// "PNGFILE2"
	int    PartWidth{ 0 };
	int    PartHeight{ 0 };
	short  FullWidth{ 0 };
	short  FullWidthThumb{ 0 };
	short  FullHeight{ 0 };
	short  FullHeightThumb{ 0 };
	int    PartSize{ 0 };		// header + pngfile
	char   FileNameWithDirectory[64]{ 0 };
	char   Unknown[16];
	int    PicOffsetX{ 0 };
	int    PicOffsetY{ 0 };
	int    PicWidth{ 0 };
	int    PicHeight{ 0 };
} PNG_PACK_TYPE2;


constexpr char PNG_PACK_TYPE2_MAGIC[8] = { 'P', 'N', 'G', 'F', 'I', 'L', 'E', '2' };
constexpr char PNG_PACK_TYPE2_START_FLAG[8] = { 'E', 'M', 'U', 'A', 'R', 'C', '_', '_' };


typedef struct
{
	char           FileId[4]; // ID of the File (must be 'T', 'I', 'M' and '2') 
	unsigned char  FormatVersion; // Version number of the format 
	unsigned char  FormatId; // ID of the format 
	unsigned short Pictures; // Number of picture data 
	char           Reserved[8]; // Padding (must be 0x00) 
} TIM2_FILEHEADER;


typedef struct
{
	unsigned long  TotalSize; // Total size of the picture data in unsigned chars 
	unsigned long  ClutSize; // CLUT data size in unsigned chars 
	unsigned long  ImageSize; // Image data size in unsigned chars 
	unsigned short HeaderSize; // Header size in unsigned chars 
	unsigned short ClutColors; // Total color number in CLUT data 
	unsigned char  PictFormat; // ID of the picture format (must be 0) 
	unsigned char  MipMapTextures;// Number of MIPMAP texture 
	unsigned char  ClutType; // Type of the CLUT data 
	unsigned char  ImageType; // Type of the Image data 
	unsigned short ImageWidth; // Width of the picture 
	unsigned short ImageHeight; // Height of the picture 
	unsigned char  GsTex0[8]; // Data for GS TEX0 register 
	unsigned char  GsTex1[8]; // Data for GS TEX1 register 
	unsigned long  GsRegs; // Data for GS TEXA, FBA, PABE register 
	unsigned long  GsTexClut; // Data for GS TEXCLUT register 
} TIM2_PICTUREHEADER;


typedef struct
{
	int                FileCount{ 0 };
} BIP_TYPE3_HEADER;

typedef struct
{
	TIM2_FILEHEADER    Tim2FileHeader;
	TIM2_PICTUREHEADER Tim2PicHeader;
	char               Magic[8]{ 0 };		// "PNGFILE3"
	int                PartWidth{ 0 };
	int                PartHeight{ 0 };
	short              FullWidth{ 0 };
	short              FullWidthThumb{ 0 };
	short              FullHeight{ 0 };
	short              FullHeightThumb{ 0 };
	int                PartSize{ 0 };		// header from magic + pngfile
	char               FileNameWithDirectory[64]{ 0 };
	char               Unknown[16];
	int                PicOffsetX{ 0 };
	int                PicOffsetY{ 0 };
	int                PicWidth{ 0 };
	int                PicHeight{ 0 };
} PNG_PACK_TYPE3;		// 出现在16字节对齐的文件偏移中