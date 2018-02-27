
#include "merge.h"
#include "picture.h"
#include "Common.h"
#include <array>
#include <assert.h>


typedef std::vector<std::pair<std::string, PNG_PACK_TYPE2>> PngGroup;


struct PngDetail
{
    Alisa::Image *Pic{ nullptr };
    int BlockOffsetX{ 0 };
    int BlockOffsetY{ 0 };
};

bool DefaultMerge(const PngGroup & pngGroup, int mergeWidth, int mergeHeight, const std::string &newName)
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
			// 位图要倒置
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


bool MergeToBase(const std::string & basePngName, const PngGroup &group1, const PngGroup &group2, const std::string & newFileName)
{
	bool success = false;

	Alisa::Image image;
	success = image.Open(basePngName);
	assert(success);

	for (auto & ele : group1)
	{
		Alisa::Image part;
		part.Open(ele.first);
		int blockOffsetX = atoi(ele.first.substr(ele.first.find('@') + 1).c_str());
		int blockOffsetY = atoi(ele.first.substr(ele.first.find('_', ele.first.find('@')) + 1).c_str());

		image.Blend(part, blockOffsetX + ele.second.PicOffsetX, blockOffsetY + ele.second.PicOffsetY);
	}
	for (auto & ele : group2)
	{
		Alisa::Image part;
		part.Open(ele.first);
		int blockOffsetX = atoi(ele.first.substr(ele.first.find('@') + 1).c_str());
		int blockOffsetY = atoi(ele.first.substr(ele.first.find('_', ele.first.find('@')) + 1).c_str());

		image.Blend(part, blockOffsetX + ele.second.PicOffsetX, blockOffsetY + ele.second.PicOffsetY);
	}

	image.SaveTo(newFileName, Alisa::E_ImageType_Png);
	return true;
}

bool MergeCharacter(const PngGroup & pngGroup, int mergeWidth, int mergeHeight, const std::string &newName)
{
	// r11 只有 1~5 共 5 个编号, 2、3 是嘴巴差分, 4、5 是眼睛差分, 按此规律直接进行组合
	// 共有 9 种组合

	std::array<PngGroup, 6> g;
	for (auto p : pngGroup)
	{
		auto name = GetFileName(p.first, true);
		name = name.substr(0, name.find(".tm2@"));
		switch (name.back())
		{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
			g[name.back() - '0'].push_back(p);
			break;
		default:
			assert(0);
			return false;
		}
	}

#ifdef _DEBUG
	// 确认一组内文件名相同
	for (size_t i = 1; i < g.size(); ++i)
	{
		if (g[i].empty()) continue;
		auto name = GetFileName(g[i].front().first, true);
		name = name.substr(0, name.find(".tm2@"));
		for (auto & ele : g[i])
		{
			auto temp = GetFileName(ele.first, true);
			assert(!name.compare(temp.substr(0, temp.find(".tm2@"))));
		}
	}
#endif

	auto basePngName = AppendFileName(newName, "_1");
	if (!DefaultMerge(g[1], mergeWidth, mergeHeight, basePngName))
		return false;

	// g[0] 为空
	MergeToBase(basePngName, g[2], g[0], AppendFileName(newName, "_2"));
	MergeToBase(basePngName, g[3], g[0], AppendFileName(newName, "_3"));
	MergeToBase(basePngName, g[4], g[0], AppendFileName(newName, "_4"));
	MergeToBase(basePngName, g[5], g[0], AppendFileName(newName, "_5"));
	MergeToBase(basePngName, g[2], g[4], AppendFileName(newName, "_6"));
	MergeToBase(basePngName, g[2], g[5], AppendFileName(newName, "_7"));
	MergeToBase(basePngName, g[3], g[4], AppendFileName(newName, "_8"));
	MergeToBase(basePngName, g[3], g[5], AppendFileName(newName, "_9"));
	return true;
}

bool Merge(const PngGroup & pngGroup, int mergeWidth, int mergeHeight, const std::string &newName)
{
	for (size_t i = 0; i < pngGroup.size(); ++i)
	{
		// 将形如 * + (S|M|L|X) + (1~5) + .tm2@ + * 的文件名视为立绘组合
		auto name = GetFileName(pngGroup[i].first, true);
		name = name.substr(0, name.find(".tm2@"));
		char b1 = name.back();
		name.pop_back();
		char b2 = name.back();
		if (b1 == '2' && (b2 == 'S' || b2 == 'M' || b2 == 'L' || b2 == 'X'))
		{
			return MergeCharacter(pngGroup, mergeWidth, mergeHeight, newName);
		}
	}

	return DefaultMerge(pngGroup, mergeWidth, mergeHeight, newName);
}
