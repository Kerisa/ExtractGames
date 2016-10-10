#pragma once

#include <vector>
#include <string>
#include <Windows.h>
#include "lpng1625/png.h"
#include "zlib-1.2.8/zlib.h"

class Merge
{
public:
    class PictureInfo
    {
    public:
        int id;
        int x;
        int y;
        int width;
        int height;
        COLORREF  * mPixels;
        COLORREF ** mLines;
        PictureInfo();
        PictureInfo(const PictureInfo &pi);
        PictureInfo & operator=(const PictureInfo &pi);
        ~PictureInfo();
    };

public:
    Merge();
    ~Merge();
    bool Initialize(const wchar_t *txtfilename, int groupnum, std::vector<int> &group);
    bool Process();
    void Release();

private:
    void GotoNextLine(char **ptr, char *limit);
    void SkipBlank(char **ptr, char *limit);
    void SaveToGroup(PictureInfo *pi);
    bool LoadFileList(const wchar_t *txtFileName);
    void SetGroup(int num, ...);
    bool OpenPng(const wchar_t *filename, PictureInfo *pi);
    bool SaveToPng(const wchar_t *filename, PictureInfo *pi);
    bool NextPermutation(std::vector<int> &v);
    void PixelsOverWrite(PictureInfo *dst, PictureInfo *src);

private:
    std::vector<int> mGroup;        // 有序排列的组ID，如mGroup[x]为第x+1组的最大ID
    std::vector<std::vector<PictureInfo>> mInfo;
    std::vector<COLORREF*> mPictureGrouped;
    std::wstring mTxtFileName;
    
    bool mGroupInitialized;
};