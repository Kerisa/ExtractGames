

#include <algorithm>
#include <cassert>
#include <direct.h>

#include <io.h>
#include <iostream>
#include <fstream>
#include <sstream>
//#include <map>
#include <vector>
#include <set>
#include <string>

#include "picture.h"
#include "FreeImage.h"


//#pragma comment(lib, "FreeImage.lib")

using namespace std;

set<string> getFiles(const string& _dir)
{
    set<string> files;//存放文件名

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
            //输出文件名
            //cout<<file.name<<endl;
            if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0 || file.attrib & _A_SUBDIR)
                continue;
            files.insert(file.name);
        }
    }
    _findclose(lf);

    return files;
}

set<string> findFileNameBeginWith(const string& baseDir, const string& name, const string& ext, set<string>& files)
{
    set<string> result;
    
    string removeAlpha(name);
    removeAlpha.pop_back();

    auto it = files.begin();
    //++it;
    // 无脑配吧
    for (; it != files.end(); ++it)
    {
        size_t find;
        if (isalpha(name.back()) || (isdigit(name.back()) && removeAlpha.back() == '_'))
            find = it->find(removeAlpha);
        else
            find = it->find(name);   // 以 base 开头

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


void MergeAndSave(const string& _baseDir, const string& _base, set<string>& files, const string& _saveDir)
{
    set<string> set1; // face
    set<string> set2; // body

    vector<char> namePart;  namePart.resize(_base.size());
    vector<char> ext;   ext.resize(_base.size());
    _splitpath_s(_base.c_str(), nullptr, 0, nullptr, 0, namePart.data(), namePart.size(), ext.data(), ext.size());

    string baseDir(_baseDir);
    if (baseDir.back() != '\\')
        baseDir += '\\';

    string base(baseDir + _base);

    assert(!_saveDir.empty());
    string saveDir(_saveDir);
    if (saveDir.back() != '\\')
        saveDir += '\\';
    int count = 1;

    for (auto& s : files)
    {
        if (s == _base)
            continue;

        if (s.find("body") != string::npos)
            set2.insert(baseDir + s);
        else if (s.find("face") != string::npos)
            set1.insert(baseDir + s);
        else
            continue;
    }

    cout << "process group:\n";
    cout << "\t" << _base << "\n";
    for (auto& s : set1)
        cout << "\t" << s << "\n";
    for (auto& s : set2)
        cout << "\t" << s << "\n";
    cout << "-------------------------------\n";

    //FreeImage_SetOutputMessage(MsgOut);

    //auto raw = FreeImage_Load(FreeImage_GetFileType(base.c_str()), base.c_str());
    //FreeImage_Save(FIF_PNG, raw, (saveDir + _base + ".png").c_str());
    //FreeImage_Unload(raw);

    Alisa::Image baseImg;
    baseImg.Open(base);
    baseImg.SaveTo(saveDir + _base + ".png", Alisa::E_ImageType_Png);


    //Alisa::Image img;
    //if (!img.Open(base))
    //{
    //    cout << "open file " << base << "failed.\n";
    //    return;
    //}

    for (auto& s1 : set1)
    {
        ostringstream oss;
        oss << count++;
        string path(saveDir + _base + oss.str() + ".png");
        //int alpha = 0;
        //auto raw = FreeImage_Load(FreeImage_GetFileType(base.c_str()), base.c_str());
        //auto raw1 = FreeImage_Load(FreeImage_GetFileType(s1.c_str()), s1.c_str());
        //auto ftype = FreeImage_GetFileType(s1.c_str());
        //
        //for (int i = 0; i < FreeImage_GetHeight(raw); ++i)
        //{
        //    FreeImage_ConvertLine24To32()
        //}

        //auto raw2 = FreeImage_ConvertToType(raw1, FreeImage_GetImageType(raw0));
        //auto type2 = FreeImage_GetColorType(raw2);
        //auto type0 = FreeImage_GetColorType(raw0);
        //FreeImage_Paste(raw0, raw2, 0, 0, alpha);
        //FreeImage_Save(FIF_PNG, raw2, path.c_str());
        //FreeImage_Save(FIF_PNG, raw0, path.c_str());
        //FreeImage_Unload(raw);
        //FreeImage_Unload(raw1);
        //FreeImage_Unload(raw2);

        Alisa::Image baseImg;
        baseImg.Open(base);

        Alisa::Image img1;
        img1.Open(s1);

        baseImg.Blend(&img1, 0, 0, Alisa::E_ImageBlendMode::E_SrcOver);

        baseImg.SaveTo(path, Alisa::E_ImageType_Png);
    }

    
    for (auto& s2 : set2)
    {
        //ostringstream oss;
        //oss << count++;
        //string path(saveDir + _base + oss.str() + ".png");

        //auto raw = FreeImage_Load(FreeImage_GetFileType(base.c_str()), base.c_str());
        //auto raw1 = FreeImage_Load(FreeImage_GetFileType(s2.c_str()), s2.c_str());
        //FreeImage_Paste(raw, raw1, 0, 0, 256);
        //FreeImage_Save(FIF_PNG, raw, path.c_str());
        //FreeImage_Unload(raw);
        //FreeImage_Unload(raw1);

        Alisa::Image baseImg;
        baseImg.Open(base);

        Alisa::Image img2;
        img2.Open(s2);

        baseImg.Blend(&img2, 0, 0, Alisa::E_ImageBlendMode::E_SrcOver);

        ostringstream oss;
        oss << count++;
        baseImg.SaveTo(saveDir + _base + oss.str() + ".png", Alisa::E_ImageType_Png);
    }

    for (auto& s2 : set2)
    {
        for (auto& s1 : set1)
        {
            //ostringstream oss;
            //oss << count++;
            //string path(saveDir + _base + oss.str() + ".png");

            //auto raw = FreeImage_Load(FreeImage_GetFileType(base.c_str()), base.c_str());
            //auto raw1 = FreeImage_Load(FreeImage_GetFileType(s1.c_str()), s1.c_str());
            //auto raw2 = FreeImage_Load(FreeImage_GetFileType(s2.c_str()), s2.c_str());
            //FreeImage_Paste(raw, raw1, 0, 0, 256);
            //FreeImage_Paste(raw, raw2, 0, 0, 256);
            //FreeImage_Save(FIF_PNG, raw, path.c_str());
            //FreeImage_Unload(raw);
            //FreeImage_Unload(raw1);
            //FreeImage_Unload(raw2);

            Alisa::Image baseImg;
            baseImg.Open(base);

            Alisa::Image img1;
            img1.Open(s1);

            Alisa::Image img2;
            img2.Open(s2);

            baseImg.Blend(&img1, 0, 0, Alisa::E_ImageBlendMode::E_SrcOver);
            baseImg.Blend(&img2, 0, 0, Alisa::E_ImageBlendMode::E_SrcOver);

            ostringstream oss;
            oss << count++;
            baseImg.SaveTo(saveDir + _base + oss.str() + ".png", Alisa::E_ImageType_Png);
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

    set<string> handled;
    auto files = getFiles(baseDir);
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
        string name, ext;
        SplitPath(filename, nullptr, nullptr, &name, &ext);
        if (string(ext.data()) == ".bmp")
        {
            auto result = findFileNameBeginWith(baseDir, name, ext, partFiles);
            if (!result.empty())
                MergeAndSave(baseDir, filename, result, saveDir);
        }
        baseFiles.erase(filename);
    }

    return 0;
}

