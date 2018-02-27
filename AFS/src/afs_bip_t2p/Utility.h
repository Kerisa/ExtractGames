#pragma once

namespace Alisa
{
    namespace Utility
    {
        template<class T>
        void SafeDelete(T** pptr)
        {
            if (pptr)
            {
                delete *pptr;
                *pptr = nullptr;
            }
        }

        template<class T>
        void SafeDeleteArray(T** pptr)
        {
            if (pptr)
            {
                delete[] * pptr;
                *pptr = nullptr;
            }
        }
    }
}