#ifndef QX_HELPERS_H
#define QX_HELPERS_H

// Standard Library Includes
#include <type_traits>

//Non-namespace Structs----------------------------------------------------------
/* TODO: Figure out how to constrain this to only accept functors, issue is at least as of C++20
 * there doesnt seem to be a way to check if a type has an arbitrary number of operator() overloads
 * with an arbitrary number of arguments.
 */
template<typename... Functors>
struct qxFuncAggregate : Functors... {
    using Functors::operator()...;
};

//Non-namespace Functions----------------------------------------------------------
template <typename T>
const T qxAsConst(T&& t) { return std::move(t); }

template <typename T>
typename std::add_const<T>::type qxAsConst(T& t) { return qAsConst(t); }

template <typename T>
void qxDelete(T*& pointer)
{
    delete pointer;
    pointer = nullptr;
}

#endif // QX_HELPERS_H
