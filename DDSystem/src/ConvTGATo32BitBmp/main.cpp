
#include "freeimage.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#pragma comment(lib, "FreeImage.lib")

using namespace std;

int main(int argc, char**argv)
{
    bool deleteFile = false;
    string paramFlag;
    string paramFileName;

    if (argc == 3)
    {
        paramFlag = argv[1];
        paramFileName = argv[2];
        deleteFile = paramFlag == "-d";
    }
    else if (argc == 2)
    {
        paramFileName = argv[1];
    }
    else
    {
        cout << "usage: Conv.exe [flag] <input file>\nflag:\n    -d, delete origin file\n";
        return -1;
    }

    FreeImage_Initialise();

    auto raw = FreeImage_Load(FIF_TARGA, paramFileName.c_str());
    if (!raw)
    {
        cout << "decode [" << paramFileName << "] failed.\n";
        return 1;
    }

    string newName(paramFileName);
    vector<char> drv;  drv.resize(newName.size());
    vector<char> path;  path.resize(newName.size());
    vector<char> name;  name.resize(newName.size());
    vector<char> ext;   ext.resize(newName.size());
    _splitpath_s(paramFileName.c_str(), drv.data(), drv.size(), path.data(), path.size(), name.data(), name.size(), ext.data(), ext.size());
    newName = string(drv.data()) + string(path.data()) + string(name.data()) + ".bmp";

    FreeImage_Save(FIF_BMP, raw, newName.c_str());
    FreeImage_Unload(raw);
    FreeImage_DeInitialise();

    if (deleteFile)
    {
        if (remove(paramFileName.c_str()) != 0)
        {
            cout << "delete [" << paramFileName << "] failed.\n";
            return 1;
        }
    }

    return 0;
}