#pragma once

#include <string>
#include <tchar.h>

namespace Alisa
{
#ifdef _UNICODE
    typedef std::wstring string_t;
#else
    typedef std::string string_t;
#endif
}