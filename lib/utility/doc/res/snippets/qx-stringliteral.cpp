//! [0]
#include <qx/utility/qx-stringliteral.h>
#include <iostream>

template<Qx::StringLiteral S>
void printString() { std::cout << "The string is: " << S.value }

int main()
{
    printString<"Hello World">();
    return 0;
}
//! [0]