//! [0]
#include <iostream>
#include <string>
#include <variant>
#include <vector>
#include <functional>
#include <qx/utility/qx-helpers.h>

// The variant to visit
using var_t = std::variant<int, long, float, double, bool, char, std::string>;

void freeFunc(char arg) { std::cout << arg << " (from wrapped char free function)"; }

int main()
{
    std::vector<var_t> vec = {10, 15l, 3.7f, 1.5, true, 'c', "hello"};

    auto f = [](std::string arg) { std::cout << arg <<  " (from string named lambda)"; };

    for (auto& v : vec)
    {
        // Visit using aggregate
        std::visit(qxFuncAggregate{
            [](auto arg) { std::cout << arg << " (from catch-all lambda)"; },
            [](float arg) { std::cout << arg << " (from float lambda)"; },
            [](double arg) { std::cout << std::fixed << arg << " (from double lambda)"; },
            [](bool arg) { std::cout << arg << " (from bool lambda)"; },
            std::function(freeFunc),
            f
        }, v);

        std::cout << std::endl;
    }
}

// 10 (from catch-all lambda)
// 15 (from catch-all lambda)
// 3.7 (from float lambda)
// 1.500000 (from double lambda)
// 1 (from bool lambda)
// c (from wrapped char free function)
// hello (from string named lambda)
//! [0]