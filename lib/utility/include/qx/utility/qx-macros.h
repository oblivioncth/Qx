#ifndef QX_MACROS_H
#define QX_MACROS_H

// Helper
/*! @cond */
#define __QX_MACRO_EXPAND1(...) __VA_ARGS__
#define __QX_MACRO_EXPAND2(...) __QX_MACRO_EXPAND1(__QX_MACRO_EXPAND1(__VA_ARGS__))
#define __QX_MACRO_EXPAND4(...) __QX_MACRO_EXPAND2(__QX_MACRO_EXPAND2(__VA_ARGS__))
#define __QX_MACRO_EXPAND8(...) __QX_MACRO_EXPAND4(__QX_MACRO_EXPAND4(__VA_ARGS__))
#define __QX_MACRO_EXPAND16(...) __QX_MACRO_EXPAND8(__QX_MACRO_EXPAND8(__VA_ARGS__))
#define __QX_MACRO_EXPAND32(...) __QX_MACRO_EXPAND8(__QX_MACRO_EXPAND8(__VA_ARGS__))
#define __QX_MACRO_EXPAND64(...) __QX_MACRO_EXPAND8(__QX_MACRO_EXPAND8(__VA_ARGS__))
#define __QX_MACRO_EXPAND128(...) __QX_MACRO_EXPAND8(__QX_MACRO_EXPAND8(__VA_ARGS__))
#define __QX_MACRO_EXPAND256(...) __QX_MACRO_EXPAND8(__QX_MACRO_EXPAND8(__VA_ARGS__))
#define __QX_MACRO_EVALUATE(...) __QX_MACRO_EXPAND256(__VA_ARGS__)

#define __QX_MACRO_CALL ()

#define __QX_FOR_EACH_NEXT() __QX_FOR_EACH_APPLY_AND_ITERATE
#define __QX_FOR_EACH_NEXT_DELIM() __QX_FOR_EACH_APPLY_AND_ITERATE_DELIM

#define __QX_FOR_EACH_APPLY_AND_ITERATE(macro, first, ...) \
    macro(first) \
    __VA_OPT__(__QX_FOR_EACH_NEXT __QX_MACRO_CALL (macro, __VA_ARGS__))

#define __QX_FOR_EACH_APPLY_AND_ITERATE_DELIM(macro, first, ...) \
    macro(first) \
    __VA_OPT__(, __QX_FOR_EACH_NEXT_DELIM __QX_MACRO_CALL (macro, __VA_ARGS__))
/*! @endcond */

// User
#define QX_SCOPED_ENUM_HASH_FUNC(T) \
inline size_t qHash(const T& t, size_t seed) { \
    return ::qHash(static_cast<typename std::underlying_type<T>::type>(t), seed); \
}

#define QSL QStringLiteral
#define QBAL QByteArrayLiteral

#define QX_FOR_EACH(macro, ...) \
    __VA_OPT__(__QX_MACRO_EVALUATE(__QX_FOR_EACH_APPLY_AND_ITERATE(macro, __VA_ARGS__)))

#define QX_FOR_EACH_DELIM(macro, ...) \
    __VA_OPT__(__QX_MACRO_EVALUATE(__QX_FOR_EACH_APPLY_AND_ITERATE_DELIM(macro, __VA_ARGS__)))

#endif // QX_MACROS_H
