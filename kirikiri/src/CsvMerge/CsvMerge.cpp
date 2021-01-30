// CsvMerge.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <fstream>
#include <set>
#include <string>
#include <vector>
#include <shlobj.h>
#include <shlwapi.h>

#include "picture.h"

#pragma comment(lib, "Shlwapi.lib")

using namespace std;

std::vector<std::string> Splite(const std::string& _str, const std::string& _delim)
{
    std::vector<std::string> out;
    size_t iLastPos = 0;
    for (size_t i = 0; i < _str.size() && iLastPos < _str.size(); )
    {
        size_t it = _str.find(_delim, iLastPos);
        if (it == std::string::npos)
          break;

        out.push_back(_str.substr(iLastPos, it - iLastPos));
        iLastPos += (it - iLastPos) + _delim.size();
    }
    if (iLastPos < _str.size())
    {
        out.push_back(_str.substr(iLastPos, _str.size() - iLastPos));
    }
    return out;
}

std::string GetFolderOfPath(const std::string & fullPath)
{
    vector<char> tmp;
    if (PathIsRelative(fullPath.c_str()))
    {
        tmp.resize(GetFullPathName(fullPath.c_str(), 0, NULL, NULL));
        GetFullPathName(fullPath.c_str(), tmp.size(), tmp.data(), NULL);
    }
    else
    {
        tmp.assign(fullPath.begin(), fullPath.end());
        tmp.push_back('\0');
    }
    if (PathRemoveFileSpecA(tmp.data()))
        return string(tmp.data()) + "\\";
    else
        return std::string();
}

std::string GetValidFileName(const std::string& in)
{
    int i = 1;
    size_t suffix = in.rfind('.');
    while (i < 1000)
    {
        std::string out = in.substr(0, suffix) + "." + std::to_string(i) + in.substr(suffix);
        std::ifstream infile;
        infile.open(out, std::ios::binary);
        if (!infile.is_open())
            return out;
        ++i;
    }
    return "";
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        cout << "usage: CsvMerge <csv file>\n";
        return 1;
    }

    string csvPath(argv[1]);
    string csvDir = GetFolderOfPath(csvPath);
    
    ifstream csv(csvPath, ios::binary);
    if (!csv.is_open())
    {
        cout << "open csv file " << csvPath << " failed\n";
        return 1;
    }

    csv.seekg(0, ios::end);
    size_t length = csv.tellg();
    csv.seekg(0, ios::beg);
    vector<char> csvData(length + 1);
    csv.read(csvData.data(), length);
    csv.close();

    if (memcmp(csvData.data(), "tag,base,diff,x,y,w,h,guidex,guidey", 35))
    {
        cout << "csv not support\n";
        return 1;
    }

    cout << "[+] valid csv file\n";
    const int columns = 9;
    auto lines = Splite(string(csvData.begin(), csvData.end()), "\r\n");
    for (size_t i = 1; i < lines.size(); ++i)
    {
        auto param = Splite(lines[i], ",");
        if (param.size() != columns)
        {
            cout << "skip invalid line [" << lines[i] << "].\n";
            continue;
        }
        if (param[2].empty())
        {
            cout << "skip base file [" << lines[i] << "].\n";
            continue;
        }

        string baseImgName = csvDir + param[1] + ".bmp";
        string diffImgName = csvDir + param[2] + ".bmp";
        string saveImgName = GetValidFileName(csvDir + param[2] + ".bmp");
        Alisa::Image bimg;
        if (!bimg.Open(baseImgName))
        {
            cout << "[-] " << baseImgName << " open failed.\n";
            continue;
        }
        Alisa::Image dimg;
        if (!dimg.Open(diffImgName))
        {
            cout << "[-] " << diffImgName << " open failed.\n";
            continue;
        }

        int x = atoi(param[4].c_str());
        int y = atoi(param[3].c_str());
        int h = atoi(param[6].c_str());
        for (int k = 0; k < h; ++k)
            bimg.CopyPixelInLine(x + k, y, &dimg, k, 0);
        if (bimg.SaveTo(saveImgName, Alisa::E_ImageType_Bmp))
            cout << "[+] save " << saveImgName << " success.\n";
        else
            cout << "[-] save " << saveImgName << " failed.\n";
    }
    cout << "[+] " << csvPath << " finished\n";
    return 0;
}

