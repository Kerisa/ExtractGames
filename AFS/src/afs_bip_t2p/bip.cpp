
#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <Windows.h>

#include "merge.h"
#include "struct.h"
#include "lzss.h"
#include "Common.h"


static std::string g_SaveDirectory;
static std::string g_TempDirectory;


bool HandleBipType2(const std::string & bipFileName, const char * const bipFileData, const char * const bipDataEnd, const char * const pngPackStart)
{
	//
	// 处理 EMUARC__ + PNGFILE2 类型的 bip 文件
	// 同时合并所有 png 图片
	//

	std::vector<std::pair<std::string, PNG_PACK_TYPE2>> pngGroup;
	
	const char *dataPtr = pngPackStart + sizeof(PNG_PACK_TYPE2_START_FLAG);

	int savedPngCount = 0;
	int mergePngWidth = 0;
	int mergePngHeight = 0;
	for (int i = 0; dataPtr < bipDataEnd; ++i)
	{
		const PNG_PACK_TYPE2 * curPng = reinterpret_cast<const PNG_PACK_TYPE2*>(dataPtr);
		const int pngDataSize = curPng->PartSize - sizeof(PNG_PACK_TYPE2);

		if (memcmp(curPng->Magic, PNG_PACK_TYPE2_MAGIC, sizeof(PNG_PACK_TYPE2_MAGIC)))
		{
			printf("invalid png file magic, index: %d, skip\n", i);
			continue;
		}

		if (!mergePngWidth || !mergePngHeight)
		{
			mergePngWidth = curPng->FullWidth;
			mergePngHeight = curPng->FullHeight;
		}
		else
		{
			assert(mergePngWidth == curPng->FullWidth);
			assert(mergePngHeight == curPng->FullHeight);
		}

		std::string saveName = g_TempDirectory + GetFileName(curPng->FileNameWithDirectory, true) + ".png";

		dataPtr += sizeof(PNG_PACK_TYPE2);
		FILE *saveFile = NULL;
		if (!fopen_s(&saveFile, saveName.c_str(), "wb"))
		{
			fwrite(dataPtr, 1, pngDataSize, saveFile);
			fclose(saveFile);
			pngGroup.push_back(std::make_pair(saveName, *curPng));
			++savedPngCount;
		}
		else
		{
			printf("save file %s failed.\n", saveName.c_str());
		}

		dataPtr += pngDataSize;
	}

	std::string mergedName = g_SaveDirectory + GetFileName(GetFileName(pngGroup[0].first, false), false) + ".png";
	if (!Merge(pngGroup, mergePngWidth, mergePngHeight, mergedName))
	{
		printf("merge failed.\n");
		return false;
	}

	return true;
}

bool ExtractPngFromBipType3(const unsigned char * const fileData, int dataLength)
{
	const unsigned char * const dataEnd = fileData + dataLength;
	const int pngFileNumber = *(int*)fileData;
	const unsigned char *dataPtr = fileData + (((1 + pngFileNumber) * sizeof(int) + 15) & ~15);

	int savedPngCount = 0;
	for (int i = 0; dataPtr < dataEnd; ++i)
	{
		const PNG_PACK_TYPE3 * curPng = (const PNG_PACK_TYPE3 *)dataPtr;
		const unsigned char *pngPicStart = dataPtr + sizeof(PNG_PACK_TYPE3);

		if (memcmp(curPng->Magic, "PNGFILE3", 8) || memcmp(curPng->Tim2FileHeader.FileId, "TIM2", 4))
		{
			printf("invalid png unit %d, stop\n", i + 1);
			break;
		}

		dataPtr += curPng->PartSize + sizeof(TIM2_FILEHEADER) + sizeof(TIM2_PICTUREHEADER);
		dataPtr = (fileData + ((int)(dataPtr - fileData + 15) & ~15));

		int pngPicSize = curPng->PartSize - (sizeof(PNG_PACK_TYPE3) - sizeof(TIM2_FILEHEADER) - sizeof(TIM2_PICTUREHEADER));
		auto saveName = g_SaveDirectory + GetFileName(curPng->FileNameWithDirectory, false) + ".png";
		FILE *saveFile = NULL;
		if (!fopen_s(&saveFile, saveName.c_str(), "wb"))
		{
			fwrite(pngPicStart, 1, pngPicSize, saveFile);
			fclose(saveFile);
			++savedPngCount;
		}
		else
		{
			printf("save file %s failed.\n", saveName.c_str());
		}
	}

	return savedPngCount > 0;
}

bool HandleBipType3(const std::string & bipFileName, const char *bipFileData, int bipFileLength)
{
	//
	// 处理 lzss 压缩的 bip/t2p 文件
	// 如果是 bip 文件，则继续将其中所有 png 图片进行分离
	//

	assert(bipFileLength > 0);

	// 文件内容为 4字节解压长度 + 数据
	int uncompressLength = *(int*)bipFileData;
	if (uncompressLength < 0)
		return false;

	unsigned char *uncompressData = new unsigned char[uncompressLength];
	bool success = false;

	do 
	{
		if (decompress_lzss(uncompressData, (unsigned char *)(bipFileData + 4), bipFileLength - 4) == uncompressLength)
		{
			std::string saveFileName(g_SaveDirectory + GetFileName(bipFileName, false));
			if (uncompressData[0] == 'T' || uncompressData[1] == 'I' || uncompressData[2] == 'M' || uncompressData[3] == '2')
			{
				saveFileName += ".tim2";

				FILE *out = NULL;
				fopen_s(&out, saveFileName.c_str(), "wb");
				fwrite(uncompressData, 1, uncompressLength, out);
				fclose(out);

				success = true;
				break;
			}
			else
			{
				saveFileName += ".Unknown";

				success = ExtractPngFromBipType3(uncompressData, uncompressLength);
				break;
			}
		}

	} while (0);

	delete[] uncompressData;
	return success;
}

int main(int argc, char ** argv)
{
	char tempPath[MAX_PATH] = { 0 };
	GetTempPath(sizeof(tempPath), tempPath);
	g_TempDirectory = tempPath;
	g_SaveDirectory = GetFilePath(argv[1], true);

	if (argc != 2)
	{
		printf("Usage: afs_bip_t2p.exe <bip/t2p file>\n");
		return 0;
	}

	FILE *bipFile = NULL;
	if (fopen_s(&bipFile, argv[1], "rb"))
	{
		printf("%s open failed.\n", argv[1]);
		return 0;
	}

	BIP_TYPE2_HEADER bipHeader;
	fread_s(&bipHeader, sizeof(bipHeader), 1, sizeof(bipHeader), bipFile);
	fseek(bipFile, 0, 2);
	int fileLength = ftell(bipFile);
	char *bipData = new char[fileLength];
	rewind(bipFile);
	fread_s(bipData, fileLength, 1, fileLength, bipFile);
	fclose(bipFile);

	// 定位 png 文件块
	// 确定 bip 文件类型
	do 
	{
		const char *pngPackStart = bipData;
		const char *bipDataEnd = bipData + fileLength;
		for (; pngPackStart < bipDataEnd - sizeof(PNG_PACK_TYPE2_START_FLAG); ++pngPackStart)
		{
			if (!memcmp(pngPackStart, PNG_PACK_TYPE2_START_FLAG, sizeof(PNG_PACK_TYPE2_START_FLAG)))
				break;
		}
		if (pngPackStart >= bipDataEnd - sizeof(PNG_PACK_TYPE2_START_FLAG))
		{
			if (!HandleBipType3(argv[1], bipData, fileLength))
			{
				printf("%s is not a valid bip or t2p file.\n", argv[1]);
			}
			break;
		}
		else
		{
			if (!HandleBipType2(argv[1], bipData, bipDataEnd, pngPackStart))
			{
				printf("%s is not a valid bip file.\n", argv[1]);
			}
			break;
		}
	} while (0);

	delete[] bipData;
	return 0;
}