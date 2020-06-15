
#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>

using namespace std;

typedef int(*UNCOM)(unsigned char * out_buf, unsigned long * out_len, unsigned char * in_buf, unsigned long in_len);

int main(int argc, char* argv[])
{
    HMODULE hZlib;
    UNCOM unCom;

    if (argc < 2)
    {
        cout << "usage: " << argv[0] << "<input> [output]\n";
        return 1;
    }

    string in(argv[1]);
    string out;
    if (argc >= 3)
    {
        out = argv[2];
    }
    else
    {
        size_t pos = in.rfind('\\');
        out = in.substr(0, pos + 1) + "un_" + in.substr(pos + 1);
    }

    if (!(hZlib = LoadLibrary(TEXT("zlib.dll"))))
    {
        cout << "miss zlib.dll.\n";
        return 1;
    }
    if (!(unCom = (UNCOM)GetProcAddress(hZlib, "uncompress")))
    {
        cout << "error zlib.dll.\n";
        return 1;
    }

    ifstream ifile(in, ios::binary);
    ifile.seekg(0, ios::end);
    vector<char> idata(ifile.tellg());
    ifile.seekg(0, ios::beg);
    ifile.read(idata.data(), idata.size());
    ifile.close();

    const size_t raw_data_offset = 5 + 8 + 8;
    uint32_t comp_size = idata.size() > raw_data_offset ? *(uint32_t*)&idata[5] : 0;
    uint32_t plain_size = idata.size() > raw_data_offset ? *(uint32_t*)&idata[13] : 0;
    if (comp_size != idata.size() - raw_data_offset)
    {
        cout << "invalid input file.\n";
        return 1;
    }

    vector<char> odata(plain_size);
    unsigned long odata_size = odata.size();
    if (unCom((PBYTE)odata.data(), &odata_size, (PBYTE)idata.data() + raw_data_offset, idata.size() - raw_data_offset))
    {
        printf("decompress failed.\r\n");
        return 1;
    }
    assert(odata_size == odata.size());
    ofstream ofile(out, ios::binary);
    ofile.write(odata.data(), odata.size());
    ofile.close();

    FreeLibrary(hZlib);
    return 0;
}