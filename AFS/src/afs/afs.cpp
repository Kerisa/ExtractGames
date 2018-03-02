

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <Windows.h>

#include "Common.h"
#include "struct.h"



int main(int argc, char ** argv)
{
    if (argc != 2)
    {
        printf("Usage: afs.exe <packet file>\n");
        return 0;
    }

    FILE *pkgFile = NULL;
    if (fopen_s(&pkgFile, argv[1], "rb"))
    {
        printf("open package failed.\n");
        return 0;
    }

    AFS_HEADER pkgHeader;
    fread_s(&pkgHeader, sizeof(pkgHeader.Magic) + sizeof(pkgHeader.FileCount),
        1, sizeof(pkgHeader.Magic) + sizeof(pkgHeader.FileCount), pkgFile);

    if (memcmp(pkgHeader.Magic, &AFS_MAGIC, sizeof(pkgHeader.Magic)))
    {
        printf("not a valid afs package.\n");
        fclose(pkgFile);
        return 0;
    }

    printf("start extracting...\n");

    std::string savePath(argv[1]);
    savePath += "_extract\\";
    CreateDirectory(savePath.c_str(), 0);

    pkgHeader.Entries = new AFS_ENTRY[pkgHeader.FileCount];
    AFS_INDEX *pkgIndices = new AFS_INDEX[pkgHeader.FileCount];

    fseek(pkgFile, 8, 0);
    fread_s(pkgHeader.Entries, sizeof(AFS_ENTRY) * pkgHeader.FileCount, 1, sizeof(AFS_ENTRY) * pkgHeader.FileCount, pkgFile);
    
    // 获取文件索引表的偏移和大小
    AFS_ENTRY pkgIndexEntry;
//    fseek(pkgFile, pkgHeader.Entries[0].Offset - sizeof(AFS_ENTRY), 0);
    fread_s(&pkgIndexEntry, sizeof(AFS_ENTRY), 1, sizeof(AFS_ENTRY), pkgFile);

    // 读取文件索引表
    fseek(pkgFile, pkgIndexEntry.Offset, 0);
    fread_s(pkgIndices, pkgIndexEntry.Length, 1, pkgIndexEntry.Length, pkgFile);

    for (int i = 0; i < pkgHeader.FileCount; ++i)
    {
        unsigned char *data = new unsigned char[pkgHeader.Entries[i].Length];
        fseek(pkgFile, pkgHeader.Entries[i].Offset, 0);
        fread_s(data, pkgHeader.Entries[i].Length, 1, pkgHeader.Entries[i].Length, pkgFile);

        FILE * saveTo = NULL;
        std::string fileName(savePath + pkgIndices[i].FileName);
        if (!memcmp(data, "OggS", 4))
        {
            fileName = ReplaceExtension(fileName, ".ogg");
        }
        else if (!memcmp(data, "RIFF", 4))
        {
            fileName = ReplaceExtension(fileName, ".wav");
        }

        if (!fopen_s(&saveTo, fileName.c_str(), "wb"))
        {
            fwrite(data, 1, pkgHeader.Entries[i].Length, saveTo);
            fclose(saveTo);

            //printf("save file %s success.\n", fileName.c_str());
        }
        else
        {
            printf("save file %s failed.\n", fileName.c_str());
        }
        delete[] data;
    }

    fclose(pkgFile);
    delete[] pkgHeader.Entries;
    delete[] pkgIndices;

    printf("extract finish.\n");

    return 0;
}