#ifndef QX_MACROS_H
#define QX_MACROS_H

#define QX_SCOPED_ENUM_HASH_FUNC(T) \
inline size_t qHash(const T& t, size_t seed) { \
    return ::qHash(static_cast<typename std::underlying_type<T>::type>(t), seed); \
}

#define QSL QStringLiteral
#define QBAL QByteArrayLiteral

#endif // QX_MACROS_H
