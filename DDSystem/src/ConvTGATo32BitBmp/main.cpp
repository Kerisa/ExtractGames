
#include "freeimage.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#pragma comment(lib, "FreeImage.lib")

using namespace std;

int main(int argc, char**argv)
{
    if (argc == 2)
    {

    }
    else if (argc == 3)
    {

    }
    else
    {
        cout << "usage.\n";
    }

    ifstream in;
    in.open(argv[1]);
    if (!in.is_open())
    {
        cout << "open [" << argv[1] << "] failed.\n";
        return 0;
    }

    FreeImage_Initialise();

    auto raw = FreeImage_Load(FIF_TARGA, argv[1]);
    if (!raw)
    {
        cout << "decode [" << argv[1] << "] failed.\n";
        return 0;
    }

    string newName(argv[1]);
    vector<char> drv;  drv.resize(newName.size());
    vector<char> path;  path.resize(newName.size());
    vector<char> name;  name.resize(newName.size());
    vector<char> ext;   ext.resize(newName.size());
    _splitpath_s(argv[1], drv.data(), drv.size(), path.data(), path.size(), name.data(), name.size(), ext.data(), ext.size());
    newName = string(drv.data()) + string(path.data()) + string(name.data()) + ".bmp";
    
    FreeImage_Save(FIF_BMP, raw, newName.c_str());
    FreeImage_Unload(raw);
    FreeImage_DeInitialise();

}