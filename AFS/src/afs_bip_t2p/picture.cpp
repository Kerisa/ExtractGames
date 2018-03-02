
#include "picture.h"
#include "libpng/png.h"
#include "Utility.h"
#include <assert.h>
#include <iterator>
#include <stdio.h>
#include <string.h>
#include <vector>

////////////////////////////////////////////////////////////////////////////////


namespace Alisa
{
    class ImageImpl
    {
    public:
        ~ImageImpl() = default;

        bool Open(const string_t & filename);
        bool NewImage(ImageInfo info);
        bool SaveTo(const string_t & filename, E_ImageType type);
        void Clear();
        ImageInfo GetImageInfo() const;
        bool Blend(const ImageImpl *obj, int offsetX, int offsetY, E_ImageBlendMode mode);
        bool CopyPixelInLine(int dstLineOffset, int dstRowOffset, ImageImpl * srcObj, int srcLineOffset, int srcRowOffset, int cnt = -1);
        void ModifyPixels(std::function<void(int row, int col, Pixel &)> func);
        void WalkPixels(std::function<void(int row, int col, const Pixel &)> func) const;
        bool StretchTo(int width, int height);
        int  OtsuThresholding() const;
        Image CreateGray() const;
        bool RemoveAlpha();
        bool AddAlpha();


    private:
        E_ImageType GetImageType(const string_t & filename);
        Pixel AlphaBlend(const Pixel & src, const Pixel & dst) const;
        Pixel SrcCopy(const Pixel & src, const Pixel & dst) const;

    private:
        ImageInfo BaseInfo;
        std::vector<std::vector<Pixel>> Pixels;
        // Pixel ** Row;

    private:
        friend class ImageCodec;
    };

    class ImageCodec
    {
    public:
        //static bool DecodeBmp(const string_t & filename, ImageImpl *img);
        //static bool EncodeBmp(const string_t & filename, const ImageImpl *img);

        static bool DecodePng(const string_t & filename, ImageImpl *img);
        static bool EncodePng(const string_t & filename, const ImageImpl *img);

        //static bool DecodeJpg(const string_t & filename, ImageImpl *img);
        //static bool EncodeJpg(const string_t & filename, const ImageImpl *img);
    };
}




////////////////////////////////////////////////////////////////////////////////////



Alisa::Image::Image()
{
    Impl = new ImageImpl;
}

Alisa::Image::Image(const Image & image)
{
}

Alisa::Image::Image(Image && image)
{
}

Alisa::Image::~Image()
{
    delete Impl;
    Impl = nullptr;
}

bool Alisa::Image::Open(const string_t & filename)
{
    return Impl->Open(filename);
}

bool Alisa::Image::NewImage(ImageInfo info)
{
    return Impl->NewImage(info);
}

bool Alisa::Image::SaveTo(const string_t & filename, E_ImageType type)
{
    return Impl->SaveTo(filename, type);
}

bool Alisa::Image::RemoveAlpha()
{
    return Impl->RemoveAlpha();
}

bool Alisa::Image::AddAlpha()
{
    return Impl->AddAlpha();
}

Alisa::ImageInfo Alisa::Image::GetImageInfo() const
{
    return Impl->GetImageInfo();
}

void Alisa::Image::Clear()
{
    return Impl->Clear();
}

bool Alisa::Image::Blend(const Image * image, int offsetX, int offsetY, E_ImageBlendMode mode)
{
    return Impl->Blend(image->Impl, offsetX, offsetY, mode);
}

bool Alisa::Image::CopyPixelInLine(int dstLineOffset, int dstRowOffset, Image * srcObj, int srcLineOffset, int srcRowOffset, int cnt)
{
    return Impl->CopyPixelInLine(dstLineOffset, dstRowOffset, srcObj->Impl, srcLineOffset, srcRowOffset, cnt);
}

void Alisa::Image::ModifyPixels(std::function<void(int row, int col, Pixel &)> func)
{
    Impl->ModifyPixels(func);
}

void Alisa::Image::WalkPixels(std::function<void(int row, int col, const Pixel &)> func) const
{
    Impl->WalkPixels(func);
}

bool Alisa::Image::StretchTo(int width, int height)
{
    return Impl->StretchTo(width, height);
}

int Alisa::Image::OtsuThresholding() const
{
    return Impl->OtsuThresholding();
}

Alisa::Image Alisa::Image::CreateGray() const
{
    return Impl->CreateGray();
}



////////////////////////////////////////////////////////////////////////////////////



bool Alisa::ImageImpl::Open(const string_t & filename)
{
    auto type = GetImageType(filename);
    switch (type)
    {
    //case E_ImageType_Bmp:
    //    return ImageCodec::DecodeBmp(filename, this);

    //case E_ImageType_Jpg:
    //    return ImageCodec::DecodeJpg(filename, this);

    case E_ImageType_Png:
        return ImageCodec::DecodePng(filename, this);

    default:
        assert(0);
        break;
    }

    return false;
}

bool Alisa::ImageImpl::NewImage(ImageInfo info)
{
    BaseInfo = info;
    Pixels.resize(info.Height);
    for (auto & line : Pixels)
        line.resize(info.Width);
    return true;
}

bool Alisa::ImageImpl::SaveTo(const string_t & filename, E_ImageType type)
{
    switch (type)
    {
    //case E_ImageType_Bmp:
    //    return ImageCodec::EncodeBmp(filename, this);

    //case E_ImageType_Jpg:
    //    return ImageCodec::EncodeJpg(filename, this);

    case E_ImageType_Png:
        return ImageCodec::EncodePng(filename, this);

    default:
        assert(0);
        break;
    }

    return false;
}

void Alisa::ImageImpl::Clear()
{
    BaseInfo.Reset();
    Pixels.clear();
}

Alisa::ImageInfo Alisa::ImageImpl::GetImageInfo() const
{
    return BaseInfo;
}

bool Alisa::ImageImpl::Blend(const ImageImpl * obj, int offsetX, int offsetY, E_ImageBlendMode mode)
{
    if (offsetX < 0 || offsetX > BaseInfo.Width - obj->BaseInfo.Width ||
        offsetY < 0 || offsetY > BaseInfo.Height - obj->BaseInfo.Height)
    {
        assert(0);
        return false;
    }

    for (size_t h = 0; h < obj->BaseInfo.Height; ++h)
    {
        for (size_t w = 0; w < obj->BaseInfo.Width; ++w)
        {
            const auto & srcPixel = obj->Pixels[h][w];
            if (srcPixel.A > 0)
            {
                auto & dstPixel = Pixels[offsetY + h][offsetX + w];

                switch (mode)
                {
                case E_AlphaBlend:
                    dstPixel = AlphaBlend(srcPixel, dstPixel);
                    break;
                case E_SrcOver:
                    dstPixel = SrcCopy(srcPixel, dstPixel);
                    break;
                default:
                    assert(0);
                    return false;
                }
            }
        }
    }

    return true;
}

bool Alisa::ImageImpl::CopyPixelInLine(int dstLineOffset, int dstRowOffset, ImageImpl * srcObj, int srcLineOffset, int srcRowOffset, int cnt)
{
    if (cnt == -1)
    {
        cnt = srcObj->BaseInfo.Width - srcRowOffset;
    }

    if (cnt <= 0)
    {
        assert(0);
        return false;
    }

    if (dstLineOffset >= BaseInfo.Height || dstRowOffset + cnt > BaseInfo.Width)
    {
        assert(0);
        return false;
    }

    if (!srcObj || srcLineOffset >= srcObj->BaseInfo.Height || srcRowOffset + cnt > srcObj->BaseInfo.Width)
    {
        assert(0);
        return false;
    }

    
    std::copy(
        stdext::make_checked_array_iterator(srcObj->Pixels[srcLineOffset].data(), srcObj->Pixels[srcLineOffset].size(), srcRowOffset),
        stdext::make_checked_array_iterator(srcObj->Pixels[srcLineOffset].data(), srcObj->Pixels[srcLineOffset].size(), srcRowOffset + cnt),
        stdext::make_checked_array_iterator(Pixels[dstLineOffset].data(), Pixels[dstLineOffset].size(), dstRowOffset)
    );
    return true;
}

void Alisa::ImageImpl::ModifyPixels(std::function<void(int row, int col, Pixel &)> func)
{
    for (size_t row = 0; row < Pixels.size(); ++row)
    {
        for (size_t col = 0; col < Pixels[row].size(); ++col)
        {
            func(row, col, Pixels[row][col]);
        }
    }
}

void Alisa::ImageImpl::WalkPixels(std::function<void(int row, int col, const Pixel &)> func) const
{
    for (size_t row = 0; row < Pixels.size(); ++row)
    {
        for (size_t col = 0; col < Pixels[row].size(); ++col)
        {
            func(row, col, Pixels[row][col]);
        }
    }
}

bool Alisa::ImageImpl::StretchTo(int width, int height)
{
    assert(0);
    return false;
}

int Alisa::ImageImpl::OtsuThresholding() const
{
    assert(0);
    return 0;
}

Alisa::Image Alisa::ImageImpl::CreateGray() const
{
    assert(0);
    return Image();
}

bool Alisa::ImageImpl::RemoveAlpha()
{
    if (BaseInfo.Component != PixelType_RGBA)
        return true;

    BaseInfo.Component = PixelType_RGB;
    for (size_t i = 0; i < Pixels.size(); ++i)
    {
        for (size_t k = 0; k < Pixels[i].size(); ++k)
        {
            Pixels[i][k].A = 0xff;
        }
    }
    return true;
}

bool Alisa::ImageImpl::AddAlpha()
{
    if (BaseInfo.Component == PixelType_RGBA)
        return true;

    assert(BaseInfo.Component == PixelType_RGB);
    if (BaseInfo.Component != PixelType_RGB)
        return false;

    BaseInfo.Component = PixelType_RGBA;
    for (size_t i = 0; i < Pixels.size(); ++i)
    {
        for (size_t k = 0; k < Pixels[i].size(); ++k)
        {
            Pixels[i][k].A = 0xff;
        }
    }

    return true;
}

Alisa::E_ImageType Alisa::ImageImpl::GetImageType(const string_t & filename)
{
    const unsigned char png_magic[] = { 0x89, 0x50, 0x4e, 0x47 };
    const unsigned char jpg_magic[] = { 0xff, 0xd8 };
    const unsigned char bmp_magic[] = { 0x42, 0x4d };


    FILE *infile = nullptr;
#ifdef _UNICODE
    errno_t err = _wfopen_s(&infile, filename.c_str(), L"rb");
#else
    errno_t err = fopen_s(&infile, filename.c_str(), "rb");
#endif
    if (err)
        return E_ImageType_Unknown;

    unsigned char buf[4];
    fread_s(buf, sizeof(buf), sizeof(buf), 1, infile);
    fclose(infile);

    E_ImageType type = E_ImageType_Unknown;

    switch (buf[0])
    {
    case 0x89:
        if (!memcmp(buf, png_magic, sizeof(png_magic)))
            type = E_ImageType_Png;
        break;

    case 0xff:
        if (jpg_magic[1] == buf[1])
            type = E_ImageType_Jpg;
        break;

    case 0x42:
        if (bmp_magic[1] == buf[1])
            type = E_ImageType_Bmp;
        break;
    }

    return type;
}

Alisa::Pixel Alisa::ImageImpl::AlphaBlend(const Pixel & src, const Pixel & dst) const
{
    float srcA = (float)src.A / 0xff;
    float dstA = (float)dst.A / 0xff;

    Pixel blend;
    blend.R = src.R * srcA + dst.R * (1 - srcA);
    blend.G = src.G * srcA + dst.G * (1 - srcA);
    blend.B = src.B * srcA + dst.B * (1 - srcA);
    blend.A = (srcA + dstA * (1 - srcA)) * 0xff;

    return blend;
}

Alisa::Pixel Alisa::ImageImpl::SrcCopy(const Pixel & src, const Pixel & dst) const
{
    return src;
}



////////////////////////////////////////////////////////////////////////////////////


bool Alisa::ImageCodec::DecodePng(const string_t & filename, ImageImpl * img)
{
    unsigned char* pPixels = nullptr;
    unsigned char** lines = nullptr;

    png_structp png_ptr;     //libpng的结构体
    png_infop   info_ptr;    //libpng的信息

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        assert(0);
        return false;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        assert(0);
        return false;
    }

    int iRetVal = setjmp(png_jmpbuf(png_ptr));
    if (iRetVal)
    {
        fprintf(stderr, "错误码：%d\n", iRetVal);
        Utility::SafeDeleteArray(&pPixels);
        Utility::SafeDeleteArray(&lines);
        assert(0);
        return false;
    }

    assert(png_ptr && info_ptr);

    FILE *infile = nullptr;
#ifdef _UNICODE
    errno_t err = _wfopen_s(&infile, filename.c_str(), L"rb");
#else
    errno_t err = fopen_s(&infile, filename.c_str(), "rb");
#endif
    if (err)
        return false;


    //
    // 绑定libpng和文件流
    //
    png_init_io(png_ptr, infile);
    png_read_info(png_ptr, info_ptr);

    //
    // 获取文件头信息
    //
    int bit_depth, color_type;
    png_uint_32 width, height;
    png_get_IHDR(png_ptr, info_ptr,
        &width, &height, &bit_depth, &color_type,
        NULL, NULL, NULL);

    //
    // 按颜色格式读取为RGBA
    //

    int pixel_byte = color_type == PNG_COLOR_TYPE_RGB ? 3 : 4;

    //要求转换索引颜色到RGB
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);
    //要求位深度强制8bit
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth<8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    //要求位深度强制8bit
    if (bit_depth == 16)
        png_set_strip_16(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);
    //灰度必须转换成RGB
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    //
    // 分配像素缓冲区
    //
    pPixels = new unsigned char[width * height * pixel_byte];
    lines = new unsigned char*[height * sizeof(unsigned char*)]; //列指针

    png_int_32 h = height - 1;
    png_int_32 i = 0;
    while (h >= 0)//逆行序读取，因为位图是底到上型
    {
        lines[i] = &pPixels[h * width * pixel_byte];
        --h;
        ++i;
    }

    //
    // 读取像素
    //
    png_read_image(png_ptr, (png_bytepp)lines);

    img->Clear();
    img->BaseInfo.Height = height;
    assert(img->BaseInfo.Height > 0);
    img->BaseInfo.Width = width;
    img->BaseInfo.Component = pixel_byte;
    

    assert((int)&((Pixel*)0)->R == 0 && (int)&((Pixel*)0)->G == 1 && (int)&((Pixel*)0)->B == 2 && (int)&((Pixel*)0)->A == 3);

    //
    // png 图像是倒置的，读取到内存后将其倒转，即 Pixels[0] 对应逻辑上的第一行
    // 如 lines[0] = 文件中的最后一行 = 对应逻辑上/显示上(屏幕坐标系)的第一行 = Pixels[0]
    //
    img->Pixels.resize(height);
    for (size_t i = 0; i < img->Pixels.size(); ++i)
    {
        img->Pixels[i].resize(width);
        for (size_t k = 0; k < img->Pixels[i].size(); ++k)
        {
            Pixel & p = img->Pixels[i][k];
            p.R = *(lines[i] + k * pixel_byte);
            p.G = *(lines[i] + k * pixel_byte + 1);
            p.B = *(lines[i] + k * pixel_byte + 2);
            p.A = pixel_byte == PixelType_RGBA ? *(lines[i] + k * pixel_byte + 3) : 0xff;
        }
    }


    //
    // 释放资源
    //
    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(infile);
    delete[] lines;
    delete[] pPixels;
    return true;
}

bool Alisa::ImageCodec::EncodePng(const string_t & filename, const ImageImpl * img)
{
    png_structp png_ptr;     //libpng的结构体
    png_infop   info_ptr;    //libpng的信息

    unsigned char** lines = nullptr;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
        goto Error;
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_write_struct(&png_ptr, NULL);
        goto Error;
    }

    if (int iRetVal = setjmp(png_jmpbuf(png_ptr)))
    {
        fprintf(stderr, "错误码：%d\n", iRetVal);
        goto Error;
    }

    assert(png_ptr && info_ptr);

    FILE *outfile = nullptr;
#ifdef _UNICODE
    errno_t err = _wfopen_s(&outfile, filename.c_str(), L"wb");
#else
    errno_t err = fopen_s(&outfile, filename.c_str(), "wb");
#endif
    if (err)
        return false;

    png_init_io(png_ptr, outfile);

    //
    //设置PNG文件头
    //
    png_set_IHDR(
        png_ptr,
        info_ptr,
        img->BaseInfo.Width,
        img->BaseInfo.Height,
        8,                              //颜色深度,
        img->BaseInfo.Component == PixelType_RGBA ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB,//颜色类型
        PNG_INTERLACE_NONE,             //不交错。交错: PNG_INTERLACE_ADAM7
        PNG_COMPRESSION_TYPE_DEFAULT,   //压缩方式
        PNG_FILTER_TYPE_DEFAULT         //什么过滤? 默认填 PNG_FILTER_TYPE_DEFAULT
    );
    //设置打包信息
    png_set_packing(png_ptr);
    //写入文件头
    png_write_info(png_ptr, info_ptr);

    lines = new unsigned char*[img->BaseInfo.Height * sizeof(unsigned char*)]; //列指针

    //
    // 同 DecodePng, 将其转置后保存
    //
    if (img->BaseInfo.Component == PixelType_RGBA)
    {
        assert((int)&((Pixel*)0)->R == 0 && (int)&((Pixel*)0)->G == 1 && (int)&((Pixel*)0)->B == 2 && (int)&((Pixel*)0)->A == 3);

        for (png_int_32 i = 0; i < img->BaseInfo.Height; ++i)
        {
            lines[i] = new unsigned char[img->BaseInfo.Width * img->BaseInfo.Component];
            memcpy_s(lines[i], img->BaseInfo.Width * img->BaseInfo.Component, img->Pixels[i].data(), img->BaseInfo.Width * sizeof(Pixel));
        }
    }
    else
    {
        for (png_int_32 i = 0; i < img->BaseInfo.Height; ++i)
        {
            lines[i] = new unsigned char[img->BaseInfo.Width * img->BaseInfo.Component];
            for (int w = 0; w < img->BaseInfo.Width; ++w)
            {
                lines[i][w * img->BaseInfo.Component]     = img->Pixels[i][w].R;
                lines[i][w * img->BaseInfo.Component + 1] = img->Pixels[i][w].G;
                lines[i][w * img->BaseInfo.Component + 2] = img->Pixels[i][w].B;
            }
        }
    }

    //
    // 写入像素
    //
    png_write_image(png_ptr, (png_bytepp)lines);
    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(outfile);
    for (int i = 0; i < img->BaseInfo.Height; ++i)
        delete[] lines[i];
    delete[] lines;

    return true;

Error:
    if (lines)
    {
        for (int i = 0; i < img->BaseInfo.Height; ++i)
            delete[] lines[i];
        delete[] lines;
    }
    assert(0);
    return false;
}