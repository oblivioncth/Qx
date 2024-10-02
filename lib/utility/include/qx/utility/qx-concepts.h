#ifndef QX_CONCEPTS_H
#define QX_CONCEPTS_H

// Standard Library Includes
#include <utility>
#include <iterator>
#include <type_traits>

// Qt Includes
#include <QHash>
#include <QMap>

/*! @cond */
namespace QxConceptsPrivate
{

template <class A, template <typename...> class B>
struct is_specialization_of : std::false_type {};

template <typename... Args, template <typename...> class B>
struct is_specialization_of<B<Args...>, B> : std::true_type {};

template <typename A, template <typename...> class B>
inline constexpr bool is_specialization_of_v = is_specialization_of<A, B>::value;

}
/*! @endcond */

namespace Qx
{

// Assignment Operators
template<class K, typename T>
concept defines_assign_for_s = requires(K klass, T type) {{ klass = type } -> std::same_as<K&>;};

template<class K, typename T>
concept defines_assign_for = requires(K klass, T type) {{ klass = type };};

template<class K>
concept defines_assign_s = defines_assign_for_s<K, K>;

template<class K>
concept defines_assign = defines_assign_for<K, K>;

template<class K, typename T>
concept defines_add_assign_for_s = requires(K klass, T type) {{ klass += type } -> std::same_as<K&>;};

template<class K, typename T>
concept defines_add_assign_for = requires(K klass, T type) {{ klass += type };};

template<class K>
concept defines_add_assign_s = defines_add_assign_for_s<K, K>;

template<class K>
concept defines_add_assign = defines_add_assign_for<K, K>;

template<class K, typename T>
concept defines_sub_assign_for_s = requires(K klass, T type) {{ klass -= type } -> std::same_as<K&>;};

template<class K, typename T>
concept defines_sub_assign_for = requires(K klass, T type) {{ klass -= type };};

template<class K>
concept defines_sub_assign_s = defines_sub_assign_for_s<K, K>;

template<class K>
concept defines_sub_assign = defines_sub_assign_for<K, K>;

template<class K, typename T>
concept defines_mult_assign_for_s = requires(K klass, T type) {{ klass *= type } -> std::same_as<K&>;};

template<class K, typename T>
concept defines_mult_assign_for = requires(K klass, T type) {{ klass *= type };};

template<class K>
concept defines_mult_assign_s = defines_mult_assign_for_s<K, K>;

template<class K>
concept defines_mult_assign = defines_mult_assign_for<K, K>;

template<class K, typename T>
concept defines_div_assign_for_s = requires(K klass, T type) {{ klass /= type } -> std::same_as<K&>;};

template<class K, typename T>
concept defines_div_assign_for = requires(K klass, T type) {{ klass /= type };};

template<class K>
concept defines_div_assign_s = defines_div_assign_for_s<K, K>;

template<class K>
concept defines_div_assign = defines_div_assign_for<K, K>;

template<class K, typename T>
concept defines_mod_assign_for_s = requires(K klass, T type) {{ klass %= type } -> std::same_as<K&>;};

template<class K, typename T>
concept defines_mod_assign_for = requires(K klass, T type) {{ klass %= type };};

template<class K>
concept defines_mod_assign_s = defines_mod_assign_for_s<K, K>;

template<class K>
concept defines_mod_assign = defines_mod_assign_for<K, K>;

template<class K, typename T>
concept defines_bit_and_assign_for_s = requires(K klass, T type) {{ klass &= type } -> std::same_as<K&>;};

template<class K, typename T>
concept defines_bit_and_assign_for = requires(K klass, T type) {{ klass &= type };};

template<class K>
concept defines_bit_and_assign_s = defines_bit_and_assign_for_s<K, K>;

template<class K>
concept defines_bit_and_assign = defines_bit_and_assign_for<K, K>;

template<class K, typename T>
concept defines_bit_or_assign_for_s = requires(K klass, T type) {{ klass |= type } -> std::same_as<K&>;};

template<class K, typename T>
concept defines_bit_or_assign_for = requires(K klass, T type) {{ klass |= type };};

template<class K>
concept defines_bit_or_assign_s = defines_bit_or_assign_for_s<K, K>;

template<class K>
concept defines_bit_or_assign = defines_bit_or_assign_for<K, K>;

template<class K, typename T>
concept defines_bit_xor_assign_for_s = requires(K klass, T type) {{ klass ^= type } -> std::same_as<K&>;};

template<class K, typename T>
concept defines_bit_xor_assign_for = requires(K klass, T type) {{ klass ^= type };};

template<class K>
concept defines_bit_xor_assign_s = defines_bit_xor_assign_for_s<K, K>;

template<class K>
concept defines_bit_xor_assign = defines_bit_xor_assign_for<K, K>;

template<class K, typename T>
concept defines_left_shift_assign_for_s = requires(K klass, T type) {{ klass <<= type } -> std::same_as<K&>;};

template<class K, typename T>
concept defines_left_shift_assign_for = requires(K klass, T type) {{ klass <<= type };};

template<class K>
concept defines_left_shift_assign_s = defines_left_shift_assign_for_s<K, K>;

template<class K>
concept defines_left_shift_assign = defines_left_shift_assign_for<K, K>;

template<class K, typename T>
concept defines_right_shift_assign_for_s = requires(K klass, T type) {{ klass >>= type } -> std::same_as<K&>;};

template<class K, typename T>
concept defines_right_shift_assign_for = requires(K klass, T type) {{ klass >>= type };};

template<class K>
concept defines_right_shift_assign_s = defines_right_shift_assign_for_s<K, K>;

template<class K>
concept defines_right_shift_assign = defines_right_shift_assign_for<K, K>;

// Increment Decrement Operators
template<class K>
concept defines_pre_increment_s = requires(K klass) {{ ++klass } -> std::same_as<K&>;};

template<class K>
concept defines_pre_increment = requires(K klass) {{ ++klass };};

template<class K>
concept defines_pre_decrement_s = requires(K klass) {{ --klass } -> std::same_as<K&>;};

template<class K>
concept defines_pre_decrement = requires(K klass) {{ --klass };};

template<class K>
concept defines_post_increment_s = requires(K klass) {{ klass++} -> std::same_as<K>;};

template<class K>
concept defines_post_increment = requires(K klass) {{ klass++ };};

template<class K>
concept defines_post_decrement_s = requires(K klass) {{ klass-- } -> std::same_as<K>;};

template<class K>
concept defines_post_decrement = requires(K klass) {{ klass-- };};

// Logical Operators
template<class K>
concept defines_negation_s = requires(K klass) {{ !klass } -> std::same_as<bool>;};

template<class K>
concept defines_negation = requires(K klass) {{ !klass };};

template<class K, typename T>
concept defines_and_for_s = requires(K klass, T type) {{ klass && type } -> std::same_as<bool>;};

template<class K, typename T>
concept defines_and_for = requires(K klass, T type) {{ klass && type };};

template<class K>
concept defines_and_s = defines_and_for_s<K, K>;

template<class K>
concept defines_and = defines_and_for<K, K>;

template<class K, typename T>
concept defines_or_for_s = requires(K klass, T type) {{ klass || type } -> std::same_as<bool>;};

template<class K, typename T>
concept defines_or_for = requires(K klass, T type) {{ klass || type };};

template<class K>
concept defines_or_s = defines_or_for_s<K, K>;

template<class K>
concept defines_or = defines_or_for<K, K>;

// Member Access Operators
template<class K, typename R, typename T>
concept defines_subscript_for_s = requires(K klass, T type) {{ klass[type] } -> std::same_as<R&>;};

template<class K, typename T>
concept defines_subscript_for = requires(K klass, T type) {{ klass[type] };};

template<class K, typename R>
concept defines_indirection_s = requires(K klass) {{ *klass } -> std::same_as<R&>;};

template<class K>
concept defines_indirection = requires(K klass) {{ *klass };};

template<class K, typename R>
concept defines_address_of_s = requires(K klass) {{ &klass } -> std::same_as<R*>;};

template<class K>
concept defines_address_of = requires(K klass) {{ &klass };};

/* TODO: Not sure how to do this one, there is a "b" parameter but its type could be anything
 * template<class K, typename R>
 * concept defines_member_ptr_s = requires(K klass, R ret) {{ klass-> } -> std::same_as<R*>;};
 */

template<class K, typename R, typename T>
concept defines_ptr_to_member_ptr_for_s = requires(K klass, T type) {{ klass->*type } -> std::same_as<R&>;};

template<class K, typename T>
concept defines_ptr_to_member_ptr_for = requires(K klass, T type) {{ klass->*type };};

// Other Operators
template<class K, typename R, typename... Types>
concept defines_call_for_s = requires(K klass, Types... args) {{ klass(args...) } -> std::same_as<R>;};

template<class K, typename... Types>
concept defines_call_for = requires(K klass, Types... args) {{ klass(args...) };};

template<class K, typename R>
concept defines_call_s = defines_call_for<K, R>;

template<class K>
concept defines_call = defines_call_for<K>;

template<class K, typename T>
concept defines_comma_for_s = requires(K klass, T type) {{ klass, type } -> std::same_as<T&>;};

template<class K, typename T>
concept defines_comma_for = requires(K klass, T type) {{ klass, type };};


// Arithmetic Operators
template<class K>
concept defines_unary_plus_s = requires(K klass) {{ +klass } -> std::same_as<K>;};

template<class K>
concept defines_unary_plus = requires(K klass) {{ +klass };};

template<class K>
concept defines_unary_minus_s = requires(K klass) {{ -klass } -> std::same_as<K>;};

template<class K>
concept defines_unary_minus = requires(K klass) {{ -klass };};

template<class K, typename T>
concept defines_add_for_s = requires(K klass, T type) {{ klass + type } -> std::same_as<K>;};

template<class K, typename T>
concept defines_add_for = requires(K klass, T type) {{ klass + type};};

template<class K>
concept defines_add_s = defines_add_for_s<K, K>;

template<class K>
concept defines_add = defines_add_for<K, K>;

template<class K, typename T>
concept defines_sub_for_s = requires(K klass, T type) {{ klass - type } -> std::same_as<K>;};

template<class K, typename T>
concept defines_sub_for = requires(K klass, T type) {{ klass - type };};

template<class K>
concept defines_sub_s = defines_sub_for_s<K, K>;

template<class K>
concept defines_sub = defines_sub_for<K, K>;

template<class K, typename T>
concept defines_mult_for_s = requires(K klass, T type) {{ klass * type } -> std::same_as<K>;};

template<class K, typename T>
concept defines_mult_for = requires(K klass, T type) {{ klass * type };};

template<class K>
concept defines_mult_s = defines_mult_for_s<K, K>;

template<class K>
concept defines_mult = defines_mult_for<K, K>;

template<class K, typename T>
concept defines_div_for_s = requires(K klass, T type) {{ klass / type } -> std::same_as<K>;};

template<class K, typename T>
concept defines_div_for = requires(K klass, T type) {{ klass / type };};

template<class K>
concept defines_div_s = defines_div_for_s<K, K>;

template<class K>
concept defines_div = defines_div_for<K, K>;

template<class K, typename T>
concept defines_mod_for_s = requires(K klass, T type) {{ klass % type } -> std::same_as<K>;};

template<class K, typename T>
concept defines_mod_for = requires(K klass, T type) {{ klass % type };};

template<class K>
concept defines_mod_s = defines_mod_for_s<K, K>;

template<class K>
concept defines_mod = defines_mod_for<K, K>;

template<class K>
concept defines_bit_not_s = requires(K klass) {{ ~klass } -> std::same_as<K>;};

template<class K>
concept defines_bit_not = requires(K klass) {{ ~klass };};

template<class K, typename T>
concept defines_bit_and_for_s = requires(K klass, T type) {{ klass & type } -> std::same_as<K>;};

template<class K, typename T>
concept defines_bit_and_for = requires(K klass, T type) {{ klass & type };};

template<class K>
concept defines_bit_and_s = defines_bit_and_for_s<K, K>;

template<class K>
concept defines_bit_and = defines_bit_and_for<K, K>;

template<class K, typename T>
concept defines_bit_or_for_s = requires(K klass, T type) {{ klass | type } -> std::same_as<K>;};

template<class K, typename T>
concept defines_bit_or_for = requires(K klass, T type) {{ klass | type };};

template<class K>
concept defines_bit_or_s = defines_bit_or_for_s<K, K>;

template<class K>
concept defines_bit_or = defines_bit_or_for<K, K>;

template<class K, typename T>
concept defines_bit_xor_for_s = requires(K klass, T type) {{ klass ^ type } -> std::same_as<K>;};

template<class K, typename T>
concept defines_bit_xor_for = requires(K klass, T type) {{ klass ^ type };};

template<class K>
concept defines_bit_xor_s = defines_bit_xor_for_s<K, K>;

template<class K>
concept defines_bit_xor = defines_bit_xor_for<K, K>;

template<class K, typename T>
concept defines_left_shift_for_s = requires(K klass, T type) {{ klass << type } -> std::same_as<K>;};

template<class K, typename T>
concept defines_left_shift_for = requires(K klass, T type) {{ klass << type };};

template<class K>
concept defines_left_shift_s = defines_left_shift_for_s<K, K>;

template<class K>
concept defines_left_shift = defines_left_shift_for<K, K>;

template<class K, typename T>
concept defines_right_shift_for_s = requires(K klass, T type) {{ klass >> type } -> std::same_as<K>;};

template<class K, typename T>
concept defines_right_shift_for = requires(K klass, T type) {{ klass >> type };};

template<class K>
concept defines_right_shift_s = defines_right_shift_for_s<K, K>;

template<class K>
concept defines_right_shift = defines_right_shift_for<K, K>;

// Comparison operators
template<class K, typename T>
concept defines_equality_for_s = requires(K klass, T type) {{ klass == type } -> std::same_as<bool>;};

template<class K, typename T>
concept defines_equality_for = requires(K klass, T type) {{ klass == type };};

template<class K>
concept defines_equality_s = defines_equality_for_s<K, K>;

template<class K>
concept defines_equality = defines_equality_for<K, K>;

template<class K, typename T>
concept defines_inequality_for_s = requires(K klass, T type) {{ klass != type } -> std::same_as<bool>;};

template<class K, typename T>
concept defines_inequality_for = requires(K klass, T type) {{ klass != type };};

template<class K>
concept defines_inequality_s = defines_inequality_for_s<K, K>;

template<class K>
concept defines_inequality = defines_inequality_for<K, K>;

template<class K, typename T>
concept defines_less_than_for_s = requires(K klass, T type) {{ klass < type } -> std::same_as<bool>;};

template<class K, typename T>
concept defines_less_than_for = requires(K klass, T type) {{ klass < type };};

template<class K>
concept defines_less_than_s = defines_less_than_for_s<K, K>;

template<class K>
concept defines_less_than = defines_less_than_for<K, K>;

template<class K, typename T>
concept defines_greater_than_for_s = requires(K klass, T type) {{ klass > type } -> std::same_as<bool>;};

template<class K, typename T>
concept defines_greater_than_for = requires(K klass, T type) {{ klass > type };};

template<class K>
concept defines_greater_than_s = defines_greater_than_for_s<K, K>;

template<class K>
concept defines_greater_than = defines_greater_than_for<K, K>;

template<class K, typename T>
concept defines_less_than_equal_for_s = requires(K klass, T type) {{ klass <= type } -> std::same_as<bool>;};

template<class K, typename T>
concept defines_less_than_equal_for = requires(K klass, T type) {{ klass <= type };};

template<class K>
concept defines_less_than_equal_s = defines_less_than_equal_for_s<K, K>;

template<class K>
concept defines_less_than_equal = defines_less_than_equal_for<K, K>;

template<class K, typename T>
concept defines_greater_than_equal_for_s = requires(K klass, T type) {{ klass >= type } -> std::same_as<bool>;};

template<class K, typename T>
concept defines_greater_than_equal_for = requires(K klass, T type) {{ klass >= type };};

template<class K>
concept defines_greater_than_equal_s = defines_greater_than_equal_for_s<K, K>;

template<class K>
concept defines_greater_than_equal = defines_greater_than_equal_for<K, K>;

template<class K, typename T>
concept defines_three_way_comp_for_s = requires(K klass, T type) {{ klass <=> type } -> std::convertible_to<std::partial_ordering>;};

template<class K, typename T>
concept defines_three_way_comp_for = requires(K klass, T type) {{ klass <=> type };};

template<class K>
concept defines_three_way_comp_s = defines_three_way_comp_for_s<K, K>;

template<class K>
concept defines_three_way_comp = defines_three_way_comp_for<K, K>;

// Trait
template<class K>
concept arithmetic = std::is_arithmetic_v<K>;

template<class K>
concept fundamental = std::is_fundamental_v<K>;

template<class K>
concept traverseable = std::bidirectional_iterator<typename K::const_iterator> &&
                       std::is_default_constructible_v<K> &&
                       requires(K klass) {{ klass.size() } -> std::integral<>;};

// Conversion
template<class K, typename T>
concept static_castable_to = requires(K klass) {{ static_cast<T>(klass) };};

// Grouping
template<class K, class ... L>
concept any_of = (std::same_as<K, L> || ...);

// Template
template<typename K, template <typename...> class L>
concept specializes = QxConceptsPrivate::is_specialization_of_v<K, L>;

// Similiar interface types
template<typename T>
concept qassociative = specializes<T, QHash> ||
                       specializes<T, QMap>;

template<typename T>
concept qcollective = specializes<T, QList> ||
                      specializes<T, QSet>;

}

#endif // QX_CONCEPTS_H
