#ifndef QX_TYPETRAITS_H
#define QX_TYPETRAITS_H

// Standard Library Includes
#include <type_traits>

namespace Qx
{

// Specialization
template <class A, template <typename...> class B>
struct is_specialization_of : std::false_type {};

template <typename... Args, template <typename...> class B>
struct is_specialization_of<B<Args...>, B> : std::true_type {};

template <typename A, template <typename...> class B>
inline constexpr bool is_specialization_of_v = is_specialization_of<A, B>::value;

// Qualifiers
template<typename T>
using target_type = std::remove_pointer_t<std::remove_reference_t<T>>;

// Misc
template<class... T>
inline constexpr bool always_false = false;

}

#endif // QX_TYPETRAITS_H
