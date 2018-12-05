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
    std::set<wchar_t> delim;
    std::transform(_delim.begin(), _delim.end(), std::inserter(delim, delim.end()), [](const std::wstring::value_type& c) { return c; });

    size_t iLastPos = 0;
    for (size_t i = 0; i < _str.size() && iLastPos < _str.size(); )
    {
        auto it = delim.find(_str[i]);
        if (it != delim.end())
        {
            if (i > iLastPos)
            {
                out.push_back(_str.substr(iLastPos, i - iLastPos));
            }

            // iLastPos移至下一个非delim字符上
            for (iLastPos = i + 1; iLastPos < _str.size() && delim.find(_str[iLastPos]) != delim.end(); ++iLastPos)
                ;
            i = iLastPos + 1;
            continue;
        }
        ++i;
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

    auto lines = Splite(csvData.data(), "\r\n");
    for (size_t i = 1; i < lines.size(); ++i)
    {
        auto param = Splite(lines[i], ",");
        if (param.size() == 8)
        {
            continue;
        }
        else if (param.size() < 9)
        {
            cout << "skip invalid line [" << lines[i] << "].\n";
            continue;
        }
        if (param[2].empty())
            continue;

        string baseImg = csvDir + param[1] + ".bmp";
        string diffImg = csvDir + param[2] + ".bmp";
        Alisa::Image bimg;
        if (!bimg.Open(baseImg))
        {
            cout << baseImg << " open failed.\n";
            return 1;
        }
        Alisa::Image dimg;
        if (!dimg.Open(diffImg))
        {
            cout << diffImg << " open failed.\n";
            return 1;
        }

        int x = atoi(param[4].c_str());
        int y = atoi(param[3].c_str());
        int h = atoi(param[6].c_str());
        for (int k = 0; k < h; ++k)
            bimg.CopyPixelInLine(x + k, y, &dimg, k, 0);
        bimg.SaveTo(diffImg, Alisa::E_ImageType_Bmp);
    }

    return 0;
}

