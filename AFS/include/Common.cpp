
#include "Common.h"

std::string GetFilePath(const std::string & origin, bool withBackslash)
{
	if (origin.find_last_of('\\') != std::string::npos)
	{
		auto filePath = origin.substr(0, origin.find_last_of('\\') + (withBackslash ? 1 : 0));
		return filePath;
	}
	else
	{
		return withBackslash ? ".\\" : ".";
	}
}

std::string GetFileName(const std::string & origin, bool withExtension)
{
	auto fileName(origin);
	if (origin.find_last_of('\\') != std::string::npos)
	{
		fileName = origin.substr(origin.find_last_of('\\') + 1);
	}

	if (!withExtension)
	{
		fileName = fileName.substr(0, fileName.find_last_of('.'));
	}
	return fileName;
}

std::string ReplaceExtension(const std::string & origin, const std::string & extension)
{
	auto fileName(origin);
	size_t slashPos = origin.find_last_of('\\');
	size_t dotPos = origin.find_last_of('.');
	if (dotPos != std::string::npos && dotPos > slashPos)
	{
		fileName.replace(dotPos, std::string::npos, extension);
	}
	else
	{
		fileName += extension;
	}

	return fileName;
}

std::string AppendFileName(const std::string & origin, const std::string & append)
{
	auto fileName(origin);
	int slashPos = origin.find_last_of('\\');
	int dotPos = origin.find_last_of('.');
	if (dotPos != std::string::npos && dotPos > slashPos)
	{
		auto ext = fileName.substr(dotPos);
		fileName.replace(dotPos, std::string::npos, append);
		fileName += ext;
	}
	else
	{
		fileName += append;
	}
	return fileName;
}