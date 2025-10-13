//! [0]
#include <qx/utility/qx-stringliteral.h>
#include <iostream>

template<Qx::StringLiteral S>
void printString()
{
    if constexpr(std::same_as<typename decltype(S)::data_t, char>)
        std::cout << "The string is: " << S.std_view();
    else if constexpr(std::same_as<typename decltype(S)::data_t, wchar_t>)
        std::wcout << "The string is: " << S.std_view();
}

int main()
{
    printString<"Hello">();
    printString<L"World">();
    return 0;
}
//! [0]
