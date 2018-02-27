
#pragma once 

#include <string>

std::string GetFilePath(const std::string & origin, bool withBackslash);
std::string GetFileName(const std::string & origin, bool withExtension);
std::string ReplaceExtension(const std::string & origin, const std::string & extension);
std::string AppendFileName(const std::string & origin, const std::string & append);
