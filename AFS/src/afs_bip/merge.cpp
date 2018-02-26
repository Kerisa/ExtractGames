
#include "merge.h"
#include "picture.h"
#include <assert.h>

struct PngDetail
{
    Alisa::Image *Pic{ nullptr };
    int BlockOffsetX{ 0 };
    int BlockOffsetY{ 0 };
};

bool Merge(const std::vector<std::pair<std::string, PNG_PACK_TYPE2>> & pngGroup, int mergeWidth, int mergeHeight, const std::string &newName)
{
    if (pngGroup.empty() || mergeHeight <= 0 || mergeWidth <= 0)
    {
        assert(0);
        return false;
    }

    std::vector<PngDetail> image;
    image.resize(pngGroup.size());
    for (int i = 0; i < image.size(); ++i)
    {
        image[i].Pic = new Alisa::Image;
        image[i].BlockOffsetX = atoi(pngGroup[i].first.substr(pngGroup[i].first.find('@') + 1).c_str());
        image[i].BlockOffsetY = atoi(pngGroup[i].first.substr(pngGroup[i].first.find('_', pngGroup[i].first.find('@')) + 1).c_str());

        image[i].Pic->Open(pngGroup[i].first);
    }

    Alisa::Image *merge = new Alisa::Image;
    Alisa::ImageInfo mergeInfo;
    mergeInfo.Height = mergeHeight;
    mergeInfo.Width = mergeWidth;
    mergeInfo.Component = image[0].Pic->GetImageInfo().Component;
    mergeInfo.FrameCount = 1;
    merge->NewImage(mergeInfo);

    int mergeBytesPerLine = mergeInfo.Width * mergeInfo.Component;
    for (int i = 0; i < image.size(); ++i)
    {
        Alisa::ImageInfo info = image[i].Pic->GetImageInfo();
		const PNG_PACK_TYPE2 * pngPack = &pngGroup[i].second;
        for (int row = 0; row < info.Height; ++row)
        {
            // Î»Í¼Òªµ¹ÖÃ
            merge->CopyPixelInLine(mergeHeight - (image[i].BlockOffsetY + pngPack->PicOffsetY) - info.Height + row, image[i].BlockOffsetX + pngPack->PicOffsetX, image[i].Pic, row, 0);
        }
    }

    merge->ModifyPixels([](int row, int col, Alisa::Pixel &p) {
        auto temp = p.R; p.R = p.B; p.B = temp;
    });

    bool success = merge->SaveTo(newName, Alisa::E_ImageType::E_ImageType_Png);

    for (int i = 0; i < image.size(); ++i)
    {
        delete image[i].Pic;
    }
    delete merge;

    return success;
}