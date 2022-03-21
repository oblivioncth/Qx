#ifndef QX_HELPERS_H
#define QX_HELPERS_H

// Standard Library Includes
#include <type_traits>

//Non-namespace Functions----------------------------------------------------------
template <typename T>
const T qxAsConst(T&& t) { return std::move(t); }

template <typename T>
typename std::add_const<T>::type qxAsConst(T& t) { return qAsConst(t); }

#endif // QX_HELPERS_H
