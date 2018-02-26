#pragma once

#include <assert.h>
#include <stdio.h>
#include <vector>


struct ImageInfo
{
public:
    int width;
    int height;
    int component;

    // 像素点的颜色分量顺序按RGB(A)排列
    // 高度与bmp一样是倒向的
    unsigned char *ppixels;

    ImageInfo() : ppixels(nullptr) { }
    ImageInfo(ImageInfo && i)
    {
        width = i.width;
        height = i.height;
        component = i.component;
        ppixels = i.ppixels;
        i.ppixels = nullptr;
    }
    ~ImageInfo()
    {
        if (ppixels)
            delete[] ppixels;
    }
private:
    ImageInfo(const ImageInfo &) { }
};



bool GetImageInfo_Png_Impl(FILE *infile, ImageInfo *pinfo);
bool GetImageRawData_Png_Impl(FILE *infile, ImageInfo *pinfo);
bool SaveToNewPicture_Png_Impl(FILE *outfile, ImageInfo *pinfo);



#include <functional>

#ifdef _UNICODE
typedef std::wstring string_t;
#else
typedef std::string string_t;
#endif

namespace Alisa
{
    class ImageInfo
    {
    public:
        ImageInfo() = default;
        ImageInfo(const ImageInfo & info) : Width(info.Width), Height(info.Height), Component(info.Component) { }
        void Reset() { Width = Height = Component = FrameCount = 0; }

        int Width{ 0 };
        int Height{ 0 };
        int Component{ 0 };
        int FrameCount{ 0 };
    };

    class Pixel
    {
    public:
        uint8_t R{ 0 };
        uint8_t G{ 0 };
        uint8_t B{ 0 };
        uint8_t A{ 0xff };
    };

    enum E_ImageType
    {
        E_ImageType_Unknown,
        E_ImageType_Bmp,
        E_ImageType_Png,
        E_ImageType_Jpg
    };

    constexpr int PixelType_RGB = 3;
    constexpr int PixelType_RGBA = 4;

    class ImageImpl;
    class Image
    {
    public:
        Image();
        Image(const Image & image);
        Image(Image && image);
        virtual ~Image();

        bool Open(const string_t & filename);
        bool NewImage(ImageInfo info);
        bool SaveTo(const string_t & filename, E_ImageType type);

        bool RemoveAlpha();
        bool AddAlpha();

        ImageInfo GetImageInfo() const;
        void Clear();

        bool CopyPixelInLine(int dstLineOffset, int dstRowOffset, Image * srcObj, int srcLineOffset, int srcRowOffset, int cnt = -1);
        void ModifyPixels(std::function<void(int row, int col, Pixel &)> func);
        void WalkPixels(std::function<void(int row, int col, const Pixel &)> func) const;

        //Image GetRawPixelData() const;
        //bool UpdateRawPixelData(const Image & image);

        bool StretchTo(int width, int height);

        int  OtsuThresholding() const;

        Image CreateGray() const;

    private:
        ImageImpl *Impl{ nullptr };
    };
}