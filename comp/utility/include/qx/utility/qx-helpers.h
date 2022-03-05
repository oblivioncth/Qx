#ifndef QX_HELPERS_H
#define QX_HELPERS_H

#include <type_traits>

//Non-namespace Functions----------------------------------------------------------
template <typename T>
typename std::add_const<T>::type qxAsConst(T& t) { return qAsConst(t); }

template <typename T>
const T qxAsConst(T&& t) { return std::move(t); }

#endif // QX_HELPERS_H
