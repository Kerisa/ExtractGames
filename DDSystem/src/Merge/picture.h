#pragma once

#include <functional>
#include <string>


#if 0
enum E_ImageType
{
    E_ImageType_Unknown,
    E_ImageType_Bmp,
    E_ImageType_Png,
    E_ImageType_Jpg
};

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
#endif

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

#if 0
int GetImageType(FILE *infile);

bool GetImageInfo_Bmp_Impl(FILE *infile, ImageInfo *pinfo);
bool GetImageInfo_Jpg_Impl(FILE *infile, ImageInfo *pinfo);
bool GetImageInfo_Png_Impl(FILE *infile, ImageInfo *pinfo);

bool GetImageRawData_Bmp_Impl(FILE *infile, ImageInfo *pinfo);
bool GetImageRawData_Jpg_Impl(FILE *infile, ImageInfo *pinfo);
bool GetImageRawData_Png_Impl(FILE *infile, ImageInfo *pinfo);

bool SaveToNewPicture_Bmp_Impl(FILE *outfile, ImageInfo *pinfo);
bool SaveToNewPicture_Jpg_Impl(FILE *outfile, ImageInfo *pinfo);
bool SaveToNewPicture_Png_Impl(FILE *outfile, ImageInfo *pinfo);

bool GetImageInfo(FILE *infile, ImageInfo *pinfo, E_ImageType type);

bool GetImageRawData(FILE *infile, ImageInfo *pinfo, E_ImageType type);
bool GetImageRawData(const wchar_t *filename, ImageInfo *pinfo);
bool GetImageRawData(const char *filename, ImageInfo *pinfo);

bool SaveToNewPicture(FILE *outfile, ImageInfo *pinfo, E_ImageType type);
bool SaveToNewPicture(const wchar_t *filename, ImageInfo *pinfo, E_ImageType type);
bool SaveToNewPicture(const char *filename, ImageInfo *pinfo, E_ImageType type);

bool StretchPixels(const ImageInfo *in, ImageInfo *out);
bool StretchPixels_Shrink(const ImageInfo *in, ImageInfo *out);
bool StretchPixels_Expand(const ImageInfo *in, ImageInfo *out);

int OtsuThresholding(const int *histogram, int total);

bool CreateGray(const ImageInfo *in, ImageInfo *out);
#endif


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