// AutoFucker.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include "dbg.h"
#include "../kirikiri/xp3.h"

using namespace std;

wstring GBKToUnicode(const string& str)
{
    int size = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, 0, 0);
    vector<wchar_t> wcs(size + 1);
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wcs.data(), wcs.size());
    return wcs.data();
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        cout << "usage: " << argv[0] << " <exe_file> <xp3_package>\n";
        return 1;
    }

    char dir[MAX_PATH] = { 0 };
    GetCurrentDirectoryA(MAX_PATH, dir);

    string xp3(argv[2]);
    string exe(argv[1]);
    string exeDir(exe.substr(0, exe.find_last_of('\\') + 1));
    if (exeDir.empty()) exeDir = string(dir) + "\\";
    string dataxp3(exeDir + "data.xp3");

    bool success = false;
    EncryptedXP3 nine;
    success = nine.Open(GBKToUnicode(xp3));
    assert(success);
    vector<file_entry> entries = nine.ExtractEntries(nine.GetPlainIndexBytes());
    
    
    nine.Close();
    success = nine.Open(GBKToUnicode(dataxp3));
    assert(success);
    vector<file_entry> entries2 = nine.ExtractEntries(nine.GetPlainIndexBytes());
    file_entry startup;    
    for (size_t i = 0; i < entries2.size(); ++i)
    {
        if (!_wcsicmp(entries2[i].file_name.c_str(), L"startup.tjs"))
        {
            startup = entries2[i];
            // break;  // 9-nine 特么的有两个
        }
    }

    assert(!startup.file_name.empty());
    string saveDir = exeDir + "[extract] " + xp3 + "\\";
    CreateDirectoryA(saveDir.c_str(), NULL);
    DebugIt(exe, exeDir, xp3, saveDir, entries, startup);
    return 0;
}

