#pragma once

#include <vector>
#include <string>

#include "struct.h"

bool Merge(const std::vector<std::pair<std::string, PNG_PACK_TYPE2>> & pngGroup, int mergeWidth, int mergeHeight, const std::string &newName);