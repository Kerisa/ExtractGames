// ycg.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <cassert>
#include <iostream>
#include <iterator>
#include <fstream>
#include <vector>
#include <string>
#include <Windows.h>

using namespace std;

typedef int(*UNCOMPRESS)(char* dst, uint32_t* dstLength, char* src, uint32_t srcLength);

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        cout << "Usage: ycg.exe ycg_file out_file\n";
        return 1;
    }

    HMODULE hZlib = LoadLibrary("zlib.dll");
    if (!hZlib)
    {
        cout << "zlib.dll not found\n";
        return 1;
    }
    UNCOMPRESS Uncompress = (UNCOMPRESS)GetProcAddress(hZlib, "uncompress");
    if (!Uncompress)
    {
        cout << "uncompress function not found in zlib.dll\n";
        CloseHandle(hZlib);
        return 1;
    }


    string infile(argv[1]);
    string outfile(argv[2]);

    ifstream in(infile, ios::binary);
    if (!in.is_open())
    {
        cout << "open " << infile << " failed.\n";
        return 1;
    }

    in.seekg(0, ios::end);
    vector<char> data(in.tellg());
    in.seekg(0, ios::beg);
    in.read(data.data(), data.size());
    in.close();


    if (memcmp(data.data(), "YCG\0", 4))
    {
        cout << "not a valid YCG file\n";
        return 1;
    }

    int width = *(int*)&data[0x4];
    int height = *(int*)&data[0x8];
    int bpp = *(int*)&data[0xc];
    int firstBlockPlainLen = *(int*)&data[0x20];
    int firstBlockPackedLen = *(int*)&data[0x24];
    int secondBlockPlainLen = *(int*)&data[0x30];
    int secondBlockPackedLen = *(int*)&data[0x34];

    vector<char> block1(firstBlockPlainLen);
    uint32_t dstLen = firstBlockPlainLen;
    if (Uncompress(block1.data(), &dstLen, &data[0x38], firstBlockPackedLen) != 0)
    {
        cout << "zlib uncompress block 1 failed\n";
        return 1;
    }

    vector<char> block2(secondBlockPlainLen);
    dstLen = secondBlockPlainLen;
    if (Uncompress(block2.data(), &dstLen, &data[0x38 + firstBlockPackedLen], secondBlockPackedLen) != 0)
    {
        cout << "zlib uncompress block 2 failed\n";
        return 1;
    }

    copy(block2.begin(), block2.end(), back_inserter(block1));
    assert(block1.size() == width * height * bpp / 8);

    ofstream out(outfile, ios::binary);
    if (!out.is_open())
    {
        cout << "create file " << outfile << " failed\n";
        return 1;
    }

    BITMAPFILEHEADER bfh{ 0 };
    bfh.bfType = 'MB';
    bfh.bfSize = block1.size() + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    BITMAPINFOHEADER bih{ 0 };
    bih.biSize = sizeof(BITMAPINFOHEADER) - 4;
    bih.biWidth = width;
    bih.biHeight = -height;
    bih.biPlanes = 1;
    bih.biBitCount = bpp;
    bih.biCompression = 0;
    bih.biSizeImage = block1.size();
    out.write((char*)&bfh, sizeof(BITMAPFILEHEADER));
    out.write((char*)&bih, sizeof(BITMAPINFOHEADER));
    
    out.write(block1.data(), block1.size());
    out.close();
    return 0;
}

