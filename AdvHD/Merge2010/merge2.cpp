
#include <algorithm>
#include <assert.h>
#include <locale>
#include <math.h>
#include "merge2.h"
#include <strsafe.h>


Merge::PictureInfo::PictureInfo()
{
    id = 0;
    x = 0;
    y = 0;
    width = 0;
    height = 0;
    mPixels = nullptr;
    mLines = nullptr;
}

Merge::PictureInfo::PictureInfo(const PictureInfo & pi)
{
    id = pi.id;
    x = pi.x;
    y = pi.y;
    width = pi.width;
    height = pi.height;
    mLines = nullptr;
    mPixels = nullptr;
        
    if (pi.mPixels)
    {
        int size = width * height * sizeof(COLORREF);
        mPixels = (COLORREF*)malloc(size);
        memcpy_s(mPixels, size, pi.mPixels, size);

        // 行索引
        size = height * sizeof(COLORREF*);
        mLines = (COLORREF**)malloc(size);
        for (int i = 0; i < height; ++i)
            mLines[i] = &mPixels[(height - 1 - i) * width];
    }
}

Merge::PictureInfo & Merge::PictureInfo::operator=(const PictureInfo & pi)
{
    if (this != &pi)
    {
        id = pi.id;
        x = pi.x;
        y = pi.y;
        width = pi.width;
        height = pi.height;
        mLines = nullptr;
        mPixels = nullptr;

        if (pi.mLines)
        {
            int size = height * sizeof(COLORREF*);
            mLines = (COLORREF**)malloc(size);
            memcpy_s(mLines, size, pi.mLines, size);
        }

        if (pi.mPixels)
        {
            int size = width * height * sizeof(COLORREF);
            mPixels = (COLORREF*)malloc(size);
            memcpy_s(mPixels, size, pi.mPixels, size);
        }
    }    
    return *this;
}

Merge::PictureInfo::~PictureInfo()
{
    if (mPixels)
        free((void *)mPixels);
    if (mLines)
        free((void *)mLines);
}

Merge::Merge()
{
    mGroupInitialized = false;

    setlocale(LC_ALL, "");
}

Merge::~Merge()
{

}

bool Merge::Initialize(const wchar_t *txtfilename, int groupnum, std::vector<int> &group)
{
    bool result = true;

    do
    {
        mGroupInitialized = false;
        
        // 获取分组信息
        mGroup.assign(group.begin(), group.end());
        mInfo.resize(groupnum);

        // 读取txt文件
        result = LoadFileList(txtfilename);
        if (!result)
            break;
        
        // 读取png文件信息
        // 去掉txt后缀名
        mTxtFileName.assign(txtfilename);
        std::size_t pos = mTxtFileName.rfind(L'.');
        mTxtFileName.assign(mTxtFileName.substr(0, pos));
        
        for (int i = 0; i<mInfo.size(); ++i)
        {
            for (int k = 0; k<mInfo[i].size(); ++k)
            {
                wchar_t tmp[32];
                StringCchPrintf(tmp, _countof(tmp), L"%02d.png", mInfo[i][k].id);
                std::wstring name(mTxtFileName);
                name += L'_';
                name += tmp;
                //StringCchPrintf(name, _countof(name), L"%s_%02d.png", mTxtFileName, mInfo[i][k].id);
                result = OpenPng(name.c_str(), &mInfo[i][k]);
                if (!result)
                {
                    assert(0);
                    return false;
                }
            }
        }

        mGroupInitialized = true;
        return true;
    } while (0);

    return false;
}

bool Merge::Process()
{
    std::vector<int> cur(mGroup.size(), -1);        // 第i组使用第cur[i]张图片, -1表示不使用
    cur[0] = 0;

    int count = 0;
    std::wstring name;
    do
    {
        // 以第一组(底图)的大小为最大范围
        PictureInfo newImage(mInfo[0][cur[0]]);
        for (int i = 1; i < mInfo.size(); ++i)
        {
            if (cur[i] == -1)
                continue;
            PictureInfo *pi = &mInfo[i][cur[i]];

            //
            // (不透明)像素覆盖
            //
            PixelsOverWrite(&newImage, pi);
        }

        wchar_t tmp[32];
        StringCchPrintf(tmp, _countof(tmp), L"%04d.png", count++);

        std::size_t pos = mTxtFileName.rfind(L'\\');
        name.assign(mTxtFileName.substr(0, pos+1));
        name += L"Merge_";
        name += mTxtFileName.substr(pos+1);
        name += L'_';
        name += tmp;
        //StringCchPrintf(name, _countof(name), L"Merge_%s_%04d.png", mTxtFileName, count++);
        SaveToPng(name.c_str(), &newImage);
        printf("%S saved.\n", name.c_str());
    } while (NextPermutation(cur));

    int total = mInfo[0].size();
    for (int i=1; i<mInfo.size(); ++i)
        total *= (mInfo[i].size() + 1);
    printf("%d pictures saved, %d in total.\n", count, total);

    return true;
}

void Merge::Release()
{
    mGroup.clear();
    std::vector<int>().swap(mGroup);
    mInfo.clear();
    std::vector<std::vector<PictureInfo>>().swap(mInfo);
}

bool Merge::LoadFileList(const wchar_t *txtFileName)
{
    if (!txtFileName)
    {
        assert(0);
        return false;
    }

    HANDLE hTxt = CreateFile(txtFileName,
        GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hTxt == INVALID_HANDLE_VALUE)
    {
        assert(0);
        return false;
    }

    DWORD size = GetFileSize(hTxt, NULL);
    DWORD bytesRead = 0;
    char* data = new char[size];
    char* dataEnd = data + size;
    bool result = ReadFile(hTxt, data, size, &bytesRead, 0);
    assert(result && size == bytesRead);
    CloseHandle(hTxt);

    if (*data > '9' || *data < '0')  // 跳过说明栏
        GotoNextLine(&data, dataEnd);

    PictureInfo pi;
    int length;
    while (data < dataEnd)
    {
        if (*data == '\n' || *data == '\r')
            ++data;
        else
        {
            pi.id = atoi(data);
            length = max(0, (int)log10((float)pi.id)) + 1;
            data += length;
            SkipBlank(&data, dataEnd);

            pi.x = atoi(data);
            length = max(0, (int)log10((float)pi.x)) + 1;
            data += length;
            SkipBlank(&data, dataEnd);

            pi.y = atoi(data);
            length = max(0, (int)log10((float)pi.y)) + 1;
            data += length;
            SkipBlank(&data, dataEnd);

            pi.width = atoi(data);
            length = max(0, (int)log10((float)pi.width)) + 1;
            data += length;
            SkipBlank(&data, dataEnd);

            pi.height = atoi(data);
            length = max(0, (int)log10((float)pi.height)) + 1;
            data += length;
            SkipBlank(&data, dataEnd);

            SaveToGroup(&pi);
        }
    }

    return true;
}

void Merge::GotoNextLine(char **ptr, char *limit)
{
    while (*ptr < limit && **ptr != '\n')
        ++(*ptr);
    if (*ptr < limit)   // ptr置于行首
        ++(*ptr);
}

void Merge::SkipBlank(char ** ptr, char * limit)
{
    while (*ptr < limit && (**ptr == ' ' || **ptr == '\t'))
        ++(*ptr);
}

void Merge::SaveToGroup(PictureInfo *pi)
{
    for (int i = 0; i < mGroup.size(); ++i)
    {
        if (pi->id <= mGroup[i])
        {
            mInfo[i].push_back(*pi);
            return;
        }
    }
    assert(0);
}

bool Merge::OpenPng(const wchar_t *filename, PictureInfo *pi)
{
    assert(pi);
    
    png_structp png_ptr;     //libpng的结构体
    png_infop   info_ptr;    //libpng的信息

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
        goto Error;
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        goto Error;
    }
    int iRetVal = setjmp(png_jmpbuf(png_ptr));//安装错误处理跳转点
                                             //当libpng内部出现错误的时候，libpng会调用longjmp直接跳转到这里运行。
    if (iRetVal)//setjmp的返回值就是libpng跳转后提供的错误代码（貌似总是1，但是还是请大家看libpng的官方文档）
    {
        fprintf(stderr, "错误码：%d\n", iRetVal);
        goto Error;
    }

    assert(png_ptr && info_ptr);

    FILE *ptrFile = NULL;
    errno_t err = _wfopen_s(&ptrFile, filename, L"rb");
    assert(err == 0 && ptrFile);

    //
    // 绑定libpng和文件流
    //
    png_init_io(png_ptr, ptrFile);
    png_read_info(png_ptr, info_ptr);

    //
    // 获取文件头信息
    //    
    int bit_depth, color_type;
    png_uint_32 width, height;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

    //
    // 按颜色格式读取为RGBA
    //

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
    COLORREF* pPixels = (COLORREF*)malloc(width * height * sizeof(COLORREF));
    COLORREF** lines = (COLORREF**)malloc(height * sizeof(COLORREF*));//列指针
    if (!lines)
        goto Error;
    png_int_32 h = height - 1;
    png_int_32 i = 0;
    while (h >= 0)//逆行序读取，因为位图是底到上型
    {
        lines[i] = (COLORREF*)&pPixels[h * width];
        --h;
        ++i;
    }

    //
    // 读取像素
    //
    png_read_image(png_ptr, (png_bytepp)lines);

    //
    // 释放资源
    //
    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(ptrFile);

    pi->mLines = lines;
    pi->mPixels = pPixels;

    return true;

Error:
    assert(0);
    return false;
}

bool Merge::SaveToPng(const wchar_t *filename, PictureInfo *pi)
{
    png_structp png_ptr;     //libpng的结构体
    png_infop   info_ptr;    //libpng的信息

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
        goto Error;
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_write_struct(&png_ptr, NULL);
        goto Error;
    }

    //安装错误处理跳转点
    //当libpng内部出现错误的时候，libpng会调用longjmp直接跳转到这里运行。
    int iRetVal = setjmp(png_jmpbuf(png_ptr));

    //setjmp的返回值就是libpng跳转后提供的错误代码（貌似总是1，但是还是请大家看libpng的官方文档）
    if (iRetVal)
    {
        fprintf(stderr, "错误码：%d\n", iRetVal);
        goto Error;
    }

    assert(png_ptr && info_ptr);

    FILE *ptrFile = NULL;
    errno_t err = _wfopen_s(&ptrFile, filename, L"wb");
    assert(err == 0);
    png_init_io(png_ptr, ptrFile);

    //
    //设置PNG文件头
    //
    png_set_IHDR(
        png_ptr,
        info_ptr,
        pi->width,
        pi->height,
        8,                              //颜色深度,
        PNG_COLOR_TYPE_RGBA,            //颜色类型, PNG_COLOR_TYPE_RGBA表示32位带透明通道真彩色
        PNG_INTERLACE_NONE,             //不交错。交错: PNG_INTERLACE_ADAM7
        PNG_COMPRESSION_TYPE_DEFAULT,   //压缩方式
        PNG_FILTER_TYPE_DEFAULT         //什么过滤? 默认填 PNG_FILTER_TYPE_DEFAULT
    );
    //设置打包信息
    png_set_packing(png_ptr);
    //写入文件头
    png_write_info(png_ptr, info_ptr);

    //
    // 写入像素
    //
    png_write_image(png_ptr, (png_bytepp)pi->mLines);
    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(ptrFile);

    return true;

Error:
    assert(0);
    return false;
}

bool Merge::NextPermutation(std::vector<int>& v)
{
    // 配合 mGroup mInfo 进行判断
    for (int i = v.size() - 1; i >= 0; --i)
    {
        if (v[i] < (int)(mInfo[i].size() - 1))
        {
            ++v[i];
            return true;
        }
        else
        {
            v[i] = -1;
            // i减1, 判断下一组
        }
    }
    return false;
}

void Merge::PixelsOverWrite(PictureInfo * dst, PictureInfo * src)
{
    for (int h = 0; h < src->height; ++h)
        for (int w = 0; w < src->width; ++w)
        {
            COLORREF srcPixel = src->mPixels[h * src->width + w];
            if ((srcPixel & 0xff000000))
            {
                // dst(底图)也有偏移, 这时要减掉
                int dstW = -dst->x + src->x + w;
                // y轴方向要倒着来, 而且因为y轴是倒着的所以dst->y要加
                int dstH = dst->y + dst->height - src->y - (src->height - h);

                // Alpha混合
                union {
                    unsigned char p[4];
                    COLORREF pixel;
                } srcP, dstP, blend;
                srcP.pixel = srcPixel;
                dstP.pixel = dst->mPixels[dstH * dst->width + dstW];

                float r[4], s[4], d[4];
                float as = (float)srcP.p[3] / 0xff, ad = (float)dstP.p[3] / 0xff;
                float fs = 1, fd = 1-as;    // SRC OVER
                s[0] = (float)srcP.p[0]; s[1] = (float)srcP.p[1];
                s[2] = (float)srcP.p[2]; s[3] = (float)srcP.p[3];
                d[0] = (float)dstP.p[0]; d[1] = (float)dstP.p[1];
                d[2] = (float)dstP.p[2]; d[3] = (float)dstP.p[3];

                for (int k = 0; k < 3; ++k)
                    r[k] = s[k]*fs + d[k]*fd;
                r[3] = (as + ad * (1 - as)) * 0xff;

                blend.p[0] = (unsigned char)r[0];
                blend.p[1] = (unsigned char)r[1];
                blend.p[2] = (unsigned char)r[2];
                blend.p[3] = (unsigned char)r[3];
                
                dst->mPixels[dstH * dst->width + dstW] = blend.pixel;
            }
        }
}
