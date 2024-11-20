#ifndef QX_HELPERS_H
#define QX_HELPERS_H

// Standard Library Includes
#include <type_traits>
#include <utility>

//Non-namespace Structs----------------------------------------------------------
/* TODO: Figure out how to constrain this to only accept functors, issue is at least as of C++20
 * there doesnt seem to be a way to check if a type has an arbitrary number of operator() overloads
 * with an arbitrary number of arguments.
 */
template<typename... Functors>
struct qxFuncAggregate : Functors... {
    using Functors::operator()...;
};

/* Explicit deduction guide. Shouldn't be needed as of C++20, but some compilers are late to the party.
 * We'd like to check for #if __cpp_deduction_guides < 201907 to see if the compiler has the feature,
 * but it seems that GCC 10.x still won't compile without this even though it reports 201907
 * implementation. This might have been an oversight, or have to due with P2082R1
 * (Fixing CTAD for aggregates) which wasn't implemented in GCC until 11.x
 */
/*! @cond */
template<class... Ts>
qxFuncAggregate(Ts...) -> qxFuncAggregate<Ts...>;
/*! @endcond */

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
