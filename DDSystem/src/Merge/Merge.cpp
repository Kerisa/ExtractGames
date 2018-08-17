

#include <algorithm>
#include <cassert>
#include <direct.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <string>

#include "picture.h"


using namespace std;

set<string> GetFiles(const string& _dir)
{
    set<string> files;

    assert(!_dir.empty());
    string dir = _dir;
    if (dir.back() != '\\')
        dir += '\\';
    dir += '*';

    _finddata_t file;
    long lf;
    //输入文件夹路径
    if ((lf = _findfirst(dir.c_str(), &file)) == -1)
    {
        cout << dir << " not found." << endl;
    }
    else
    {
        while (_findnext(lf, &file) == 0) {
            if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0 || file.attrib & _A_SUBDIR)
                continue;
            files.insert(file.name);
        }
    }
    _findclose(lf);

    return files;
}

set<string> findFileNameBeginWith(const string& baseDir, const string& nameWithoutExt, set<string>& files)
{
    set<string> result;

    string removeAlpha(nameWithoutExt);
    removeAlpha.pop_back();

    // 从文件名中无法推测所有情况，还是无脑配吧

    for (auto it = files.begin(); it != files.end(); ++it)
    {
        size_t find;
        if (isalpha(nameWithoutExt.back()) || (isdigit(nameWithoutExt.back()) && removeAlpha.back() == '_'))
            find = it->find(removeAlpha);
        else
            find = it->find(nameWithoutExt);   // 以 base 开头

        if (find != string::npos)
        {
            auto facePos = it->find("face");
            auto bodyPos = it->find("body");
            if (facePos != string::npos)
            {
                // 差分中没有字母则认为可匹配
                //auto sub = it->substr(removeAlpha.size(), facePos - removeAlpha.size());  // abcd..等标号, base 中至多有一个字母标号
                //if (sub.empty() || sub.find_first_of(alphaInBase) != string::npos)
                    result.insert(*it);
            }
            if (bodyPos != string::npos)
            {
                //auto sub = it->substr(removeAlpha.size(), bodyPos - removeAlpha.size());  // abcd..等标号, base 中至多有一个字母标号
                //if (sub.find_first_of(alphaInBase) != string::npos)
                    result.insert(*it);
            }
        }
    }
    return result;
}


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


void MergeAndSave(const string& _baseDir, const string& _baseNameWithExt, set<string>& files, const string& _saveDir)
{
    set<string> set1; // face
    set<string> set2; // body

    string namePart, ext;
    SplitPath(_baseNameWithExt, nullptr, nullptr, &namePart, &ext);

    assert(!_baseDir.empty() && _baseDir.back() == '\\');
    assert(!_saveDir.empty() && _saveDir.back() == '\\');

    string baseFullPath(_baseDir + _baseNameWithExt);

    int count = 1;

    for (auto& s : files)
    {
        if (s.find("body") != string::npos)
            set2.insert(_baseDir + s);
        else if (s.find("face") != string::npos)
            set1.insert(_baseDir + s);
    }

    cout << "process group:\n";
    cout << "\t" << _baseDir << _baseNameWithExt << "\n";
    for (auto& s : set1)
        cout << "\t" << s << "\n";
    for (auto& s : set2)
        cout << "\t" << s << "\n";
    cout << "-------------------------------\n";

    // copy
    Alisa::Image baseImg;
    baseImg.Open(baseFullPath);
    baseImg.SaveTo(_saveDir + _baseNameWithExt + ".png", Alisa::E_ImageType_Png);

    for (auto& s1 : set1)
    {
        ostringstream oss;
        oss << count++;
        string path(_saveDir + _baseNameWithExt + oss.str() + ".png");

        Alisa::Image baseImg, img1;
        baseImg.Open(baseFullPath);
        img1.Open(s1);
        baseImg.Blend(&img1, 0, 0, Alisa::E_ImageBlendMode::E_SrcOver);
        baseImg.SaveTo(path, Alisa::E_ImageType_Png);
    }

    for (auto& s2 : set2)
    {
        ostringstream oss;
        oss << count++;
        string path(_saveDir + _baseNameWithExt + oss.str() + ".png");

        Alisa::Image baseImg, img2;
        baseImg.Open(baseFullPath);
        img2.Open(s2);
        baseImg.Blend(&img2, 0, 0, Alisa::E_ImageBlendMode::E_SrcOver);
        baseImg.SaveTo(path, Alisa::E_ImageType_Png);
    }

    for (auto& s2 : set2)
    {
        for (auto& s1 : set1)
        {
            ostringstream oss;
            oss << count++;
            string path(_saveDir + _baseNameWithExt + oss.str() + ".png");

            Alisa::Image baseImg, img1, img2;
            baseImg.Open(baseFullPath);
            img1.Open(s1);
            img2.Open(s2);
            baseImg.Blend(&img1, 0, 0, Alisa::E_ImageBlendMode::E_SrcOver);
            baseImg.Blend(&img2, 0, 0, Alisa::E_ImageBlendMode::E_SrcOver);
            baseImg.SaveTo(path, Alisa::E_ImageType_Png);
        }
    }
}



int main(int argc, char** argv)
{
    if (argc != 3)
    {
        cout << "usage: Merge.exe <input dir> <output dir>\n";
        return 0;
    }


    string baseDir(argv[1]);
    if (baseDir.back() != '\\')
        baseDir += '\\';

    string saveDir(argv[2]);
    if (saveDir.back() != '\\')
        saveDir += '\\';


    auto files = GetFiles(baseDir);
    set<string> baseFiles;
    set<string> partFiles;
    for (auto& f : files)
    {
        Alisa::Image img;
        if (img.Open(baseDir + f))
        {
            if (img.GetImageInfo().Component == 3)
                baseFiles.insert(f);
            else if (img.GetImageInfo().Component == 4)
                partFiles.insert(f);
        }
    }

    string cmd("mkdir \"" + saveDir + "\"");
    system(cmd.c_str());

    while (!baseFiles.empty())
    {
        auto filename = *baseFiles.begin();
        string name;
        SplitPath(filename, nullptr, nullptr, &name, nullptr);
        auto result = findFileNameBeginWith(baseDir, name, partFiles);
        if (!result.empty())
            MergeAndSave(baseDir, filename, result, saveDir);
        baseFiles.erase(filename);
    }

    return 0;
}

