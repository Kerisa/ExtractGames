
#include "stdafx.h"
#include "dbgpal.h"
#include <future>
#include <memory>

using namespace std;

int main(int argc, char** argv)
{
    if (argc < 5)
    {
        cout << "usage:\n    Extract.exe <Flag> <GameID> <GameEXEPath> <SaveDir> <ImageListTXTFile...>\n"
             << "    Flag:\n"
             << "        -y\tskip starting prompt\n"
             << "    GameID:\n"
             << "        1 : ナツイロココロログ\n"
             << "        2 : ナツイロココロログ ～Happy Summer～\n";
        return 0;
    }

    bool skip;
    int txtFileStartIdx;
    string exePath, saveDir;
    GameVersion gameVer;
    if (!_stricmp(argv[1], "-y"))
    {
        skip = true;
        gameVer = static_cast<GameVersion>(atoi(argv[2]));
        exePath = argv[3];
        saveDir = argv[4];
        txtFileStartIdx = 5;
    }
    else
    {
        skip = false;
        gameVer = static_cast<GameVersion>(atoi(argv[1]));
        exePath = argv[2];
        saveDir = argv[3];
        txtFileStartIdx = 4;
    }

    if (!IsValidGameVersion(gameVer))
    {
        cout << "error game version.\n";
        return 0;
    }

    setlocale(LC_ALL, "zh");

    saveDir = FullPath(saveDir);
    if (!saveDir.empty() && saveDir.back() != '\\')
        saveDir += '\\';
    system((string("mkdir \"") + saveDir + "\"").c_str());


    string drv, path;
    SplitPath(exePath, &drv, &path, nullptr, nullptr);
    string exeDir = drv + path;

    if (!skip)
    {
        cout << "press enter after exit game<Enter>";
        getchar();
    }

    vector<ImageInfo> imageFileList;
    for (int i = txtFileStartIdx; i < argc; ++i)
    {
        ifstream in;
        in.open(argv[i]);
        if (!in.is_open())
        {
            cout << "txt file [" << argv[i] << "] open failed.\n";
            return 0;
        }
        string line;
        std::getline(in, line); // skip comment line
        while (std::getline(in, line))
        {
            vector<string> result;
            Splite(line.c_str(), "\t", result);
            if (!result.empty())
            {
                if (result[0].rfind(".pgd") != string::npos || result[0].rfind(".PGD") != string::npos)
                {
                    result[0] = result[0].substr(0, result[0].size() - 4);
                    imageFileList.push_back(ImageInfo(result[0], atoi(result[1].c_str())));
                }
            }
        }

    }

    while (1)
    {
        int count = std::count_if(imageFileList.begin(), imageFileList.end(), [](ImageInfo& ii) {return ii.handled; });
        if (count >= imageFileList.size())
            break;

        auto task = std::make_shared<std::packaged_task<bool()>>(
            [&]() { return DebugIt(exePath, exeDir, saveDir, imageFileList, gameVer); }
        );
        std::future<bool> ret = task->get_future();
        // 调式器需要重新启动线程
        auto th = std::thread([task]() { (*task)(); });
        th.join();
        if (!ret.get())
        {
            cout << "error occured.\n";
            break;
        }
    }

    printf("finish.\n");
    return 0;
}