
#include <iostream>
#include <fstream>

#include "pal.h"

using namespace std;

void SplitPath(const string& in, string* drv, string* path, string* name, string* ext)
{
    vector<char> _drv;   _drv.resize(in.size());
    vector<char> _path;  _path.resize(1024);
    vector<char> _name;  _name.resize(in.size());
    vector<char> _ext;   _ext.resize(in.size());

    _splitpath_s(in.c_str(), _drv.data(), _drv.size(), _path.data(), _path.size(), _name.data(), _name.size(), _ext.data(), _ext.size());

    if (drv) *drv = _drv.data();
    if (path) *path = _path.data();
    if (name) *name = _name.data();
    if (ext) *ext = _ext.data();
}


int main(int argc, char** argv)
{
    if (argc != 3)
    {
        cout << "usage:\n    unpackPAC <pac path> <save dir>.\n";
        return 0;
    }

    PAL p;
    if (!p.Open(argv[1]))
    {
        cout << argv[1] << " open failed.\n";
        return 0;
    }

    if (!p.ExtractEntries())
    {
        cout << "extract entries failed.\n";
        return 0;
    }

    string drv, path, name;
    SplitPath(argv[1], &drv, &path, &name, nullptr);
    string outName = drv + path + name + "_list.txt";
    ofstream out;
    out.open(outName);
    if (!out.is_open())
    {
        cout << outName << " create failed.\n";
        return 0;
    }

    out << "# name\ttype\textra\n";
    for (auto& e : p.mEntries)
    {
        out << e.e.Filename << '\t' << (int)e.Type << '\t' << e.Extra << '\n';
        //out.write(e.e.Filename, strlen(e.e.Filename));
        //out.write("\n", 1);
    }
    out.close();
    cout << outName << " successfully saved.\n";

    string dir(argv[2]);
    if (dir.back() != '\\')
        dir += '\\';
    if (!p.ExtractResource(dir))
    {
        cout << "some error occurred.\n";
    }
    return 0;
}