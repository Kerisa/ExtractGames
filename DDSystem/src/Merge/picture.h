#pragma once

#include <functional>
#include <string>

#pragma pack(push)
#pragma pack(1)
typedef struct
{
    unsigned short bfType;
    unsigned long  bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned long  bfOffBits;
} _BITMAPFILEHEADER;

typedef struct {
    unsigned long  biSize;
    long           biWidth;
    long           biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned long  biCompression;
    unsigned long  biSizeImage;
    long           biXPelsPerMeter;
    long           biYPelsPerMeter;
    unsigned long  biClrUsed;
    unsigned long  biClrImportant;
} _BITMAPINFOHEADER;

#pragma pack(pop)


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

    enum E_ImageBlendMode
    {
        E_SrcOver,
        E_AlphaBlend,
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

        static ImageInfo GetImageInfo(const string_t & filename);
        ImageInfo GetImageInfo() const;
        const std::vector<std::vector<Pixel>> & GetPixelsGroup() const;
        void Clear();

        bool Blend(const Image *image, int offsetX, int offsetY, E_ImageBlendMode mode);

        bool CopyPixelInLine(int dstLineOffset, int dstRowOffset, Image * srcObj, int srcLineOffset, int srcRowOffset, int cnt = -1);
        void ModifyPixels(std::function<void(int row, int col, Pixel &)> func);
        void WalkPixels(std::function<void(int row, int col, const Pixel &)> func) const;

        //Image GetRawPixelData() const;
        //bool UpdateRawPixelData(const Image & image);

        bool StretchTo(Image *dst, int width, int height) const;

        int  OtsuThresholding() const;

        Image CreateGray() const;

    private:
        ImageImpl *Impl{ nullptr };
    };
}