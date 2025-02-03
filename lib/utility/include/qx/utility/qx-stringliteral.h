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

template <size_t N>
struct StringLiteral16
{
    char16_t value[N];

    constexpr StringLiteral16(const char16_t (&str)[N])
    {
        std::copy_n(str, N, value);
    }
};

}

#endif // QX_STRINGLITERAL_H
