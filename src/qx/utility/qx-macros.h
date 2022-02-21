#ifndef QX_MACROS_H
#define QX_MACROS_H

#define QX_SCOPED_ENUM_HASH_FUNC(T) \
inline uint qHash(const T& t, uint seed) { \
    return ::qHash(static_cast<typename std::underlying_type<T>::type>(t), seed); \
}

#endif // QX_MACROS_H