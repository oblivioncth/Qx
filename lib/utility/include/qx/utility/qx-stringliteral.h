#ifndef QX_STRINGLITERAL_H
#define QX_STRINGLITERAL_H

#include <algorithm>

namespace Qx
{

template<size_t N>
struct StringLiteral
{
//-Instance Fields---------------------------------------------------------------------------------------------------
    char value[N];

//-Constructor----------------------------------------------------------------------------------------------------------
    constexpr StringLiteral(const char (&str)[N])
    {
        std::copy_n(str, N, value);
    }
};

}

#endif // QX_STRINGLITERAL_H
