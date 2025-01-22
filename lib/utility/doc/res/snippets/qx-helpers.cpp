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

//! [1]
template<typename T>
class Container
{
    T mT;
public:
    Container(const T& v) : mT(v) {}

    auto operator->() const requires Qx::arrowable_container_type<T>
    {
        return Qx::container_arrow_operator(mT);
    }

    auto operator->() requires Qx::arrowable_container_type<T>
    {
        return Qx::container_arrow_operator(mT);
    }
};

struct Foo
{
    int data = -1;
    void print() const { std::cout << "Const " << data << std::endl; } 
    void print() { std::cout << "Non-const " << data << std::endl; } 
};

int main() {
    // Helper
    Foo fooForPtr{4};
    Foo fooForRef{5};
    Foo eight{8};
    const Foo* fooForPtrRef{&eight};
    int ten{10};

    // Demo
    Foo f{0};
    Container<Foo> cf(Foo{1});
    Container<std::optional<Foo>> cof(Foo{2});
    Container<std::shared_ptr<Foo>> csf = std::make_shared<Foo>(3);
    Container<Foo*> cpf = &fooForPtr;
    Container<Foo&> crf = fooForRef;
    const Container<Foo> ccf(Foo{6});
    Container<const Foo> ccf2(Foo{7});
    Container<const Foo*&> ccfpr = fooForPtrRef;
    Container<int> ci = 9;
    Container<int*> cpi = &ten;

    cf->print();
    cof->print();
    csf->print();
    cpf->print();
    crf->print();
    ccf->print();
    ccf2->print();
    ccfpr->print();
    //ci->print(); Constraints not satisfied, int doesn't make sense for operator->()
    //cpi-print(); Constraints not satisfied, int doesn't make sense for operator->()
}

//Output:
//Non-const 1
//Non-const 2
//Non-const 3
//Non-const 4
//Non-const 5
//Const 6
//Const 7
//Const 8
//! [1]