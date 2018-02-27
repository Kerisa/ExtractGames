
#include <stdio.h>
#include <string>
#include "struct.h"
#include "Common.h"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("Usage: afs_tim2.exe <tim2 file>\n");
		return 0;
	}

	FILE *in = NULL;
	if (fopen_s(&in, argv[1], "rb"))
	{
		printf("open %s failed.\n", argv[1]);
		return 0;
	}

	fseek(in, 0, 2);
	const int fileLength = ftell(in);
	char * fileData = new char[fileLength];
	rewind(in);
	fread_s(fileData, fileLength, 1, fileLength, in);
	fclose(in);

	const TIM2_FILEHEADER *tim2FileHeader = (TIM2_FILEHEADER *)fileData;
	const TIM2_PICTUREHEADER *tim2PicHeader = (TIM2_PICTUREHEADER *)(fileData + sizeof(TIM2_FILEHEADER));

	if (fileLength < sizeof(TIM2_FILEHEADER) + sizeof(TIM2_PICTUREHEADER) ||
		memcmp(tim2FileHeader->FileId, "TIM2", 4))
	{
		printf("%s is not a valid tim2 file.\n", argv[1]);
		delete[] fileData;
		return 0;
	}

	char *tim2ImageStart = fileData + sizeof(TIM2_FILEHEADER) + sizeof(TIM2_PICTUREHEADER);

	if (!memcmp(tim2ImageStart, "PNGFILE3", 8))
	{
		// 内含 png 文件
		const PNG_PACK_TYPE3 *png = (PNG_PACK_TYPE3 *)fileData;

		std::string fileName = ReplaceExtension(argv[1], ".png");
		FILE *out;
		if (fopen_s(&out, fileName.c_str(), "wb"))
		{
			printf("create file %s failed.\n", fileName.c_str());
			delete[] fileData;
			return 0;
		}
		int pngPicSize = png->PartSize - (sizeof(PNG_PACK_TYPE3) - sizeof(TIM2_FILEHEADER) - sizeof(TIM2_PICTUREHEADER));
		fwrite(fileData + sizeof(PNG_PACK_TYPE3), 1, pngPicSize, out);
		fclose(out);
	}
	else
	{
		// tim2 像素格式为 4 字节 argb

		for (auto ptr = tim2ImageStart; ptr < fileData + fileLength; ptr += 4)
		{
			// r 与 b 互换
			const char temp = ptr[0];
			ptr[0] = ptr[2];
			ptr[2] = temp;
		}

		BITMAP_FILE_HEADER bmpFileHeader;
		BITMAP_INFO_HEADER bmpPicHeader;
		bmpFileHeader.bfType = 'MB';
		bmpFileHeader.bfSize = sizeof(BITMAP_FILE_HEADER) + sizeof(BITMAP_INFO_HEADER) + tim2PicHeader->ImageSize;
		bmpFileHeader.bfOffBits = sizeof(BITMAP_FILE_HEADER) + sizeof(BITMAP_INFO_HEADER);

		bmpPicHeader.biSize = sizeof(BITMAP_INFO_HEADER);
		bmpPicHeader.biWidth = tim2PicHeader->ImageWidth;
		bmpPicHeader.biHeight = tim2PicHeader->ImageHeight;
		bmpPicHeader.biBitCount = 32;
		bmpPicHeader.biSizeImage = tim2PicHeader->ImageSize;

		std::string fileName = ReplaceExtension(argv[1], ".bmp");
		FILE *out;
		if (fopen_s(&out, fileName.c_str(), "wb"))
		{
			printf("create file %s failed.\n", fileName.c_str());
			delete[] fileData;
			return 0;
		}

		fwrite(&bmpFileHeader, 1, sizeof(BITMAP_FILE_HEADER), out);
		fwrite(&bmpPicHeader, 1, sizeof(BITMAP_INFO_HEADER), out);
		// 倒置
		int bytesPerLine = tim2PicHeader->ImageWidth * 4;
		for (int i = tim2PicHeader->ImageHeight - 1; i >= 0; --i)
		{
			fwrite(tim2ImageStart + i * bytesPerLine, 1, bytesPerLine, out);
		}
		fclose(out);
	}

	delete[] fileData;
	return 0;
}