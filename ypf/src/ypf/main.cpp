// ypf.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ypf.h"
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char** argv)
{
    if (argc != 2 && argc != 3)
    {
        cout << "usage: ypf.exe ypf_pack [out_dir]\n";
        return 1;
    }

    string packet(argv[1]);
    string dir = argc == 3 ? argv[2] : ".";
    if (!dir.empty() && dir.back() != '\\')
        dir += '\\';

    YPF ypf;
    if (!ypf.Open(argv[1]))
    {
        cout << argv[1] << "打开失败\n";
        return 1;
    }

    if (!ypf.ExtractEntries())
    {
        cout << "获取文件列表失败\n";
        return 1;
    }
    
    if (!ypf.ExtractResource(dir))
    {
        cout << "解压文件失败\n";
        return 1;
    }

    ypf.Close();
    return 0;
}