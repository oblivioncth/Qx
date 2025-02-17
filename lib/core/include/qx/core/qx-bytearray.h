#ifndef QX_BYTEARRAY_H
#define QX_BYTEARRAY_H

// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Standard Library Includes
#include <concepts>

// Qt Includes
#include <QByteArray>
#include <QtEndian>

// Extra-component Includes
#include "qx/utility/qx-concepts.h"

namespace Qx
{
	
class QX_CORE_EXPORT ByteArray
{
//-Class Functions----------------------------------------------------------------------------------------------
public:
    template<typename T>
        requires arithmetic<T>
    static QByteArray fromPrimitive(T primitive, QSysInfo::Endian endianness = QSysInfo::ByteOrder)
    {
        // Ensure correct byte order
        if(endianness == QSysInfo::LittleEndian)
            primitive = qToLittleEndian(primitive);
        else
            primitive = qToBigEndian(primitive);

        // Return QByteArray constructed from primitive viewed as a char array
        return QByteArray(reinterpret_cast<const char*>(&primitive), sizeof(T));
    }

    /*
     * This is valid C++17 syntax for explicit template specialization, but due to an outstanding
     * bug this won't compile with GCC: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85282
     *
     * The workaround is to fake partial template specialization using a dummy template parameter.
     */
#if defined __GNUC__ && !defined __clang__ // If using G++
    template<typename>
    inline QByteArray fromPrimitive(bool primitive, QSysInfo::Endian endianness)
#else
    template<>
    inline QByteArray fromPrimitive<bool>(bool primitive, QSysInfo::Endian endianness)
#endif
    {
        Q_UNUSED(endianness);
        // Ensures true -> 0x01 and false -> 0x00
        return primitive ? QByteArray(1, '\x01') : QByteArray(1, '\x00');
    }

    template<typename T>
        requires arithmetic<T>
    static T toPrimitive(QByteArray ba, QSysInfo::Endian endianness = QSysInfo::ByteOrder)
    {
        if(endianness == QSysInfo::LittleEndian)
        {
            if(ba.size() < sizeof(T))
                ba.append(sizeof(T) - ba.size(),'\x00');
            return qFromLittleEndian<T>(ba);
        }
        else
        {
            if(ba.size() < sizeof(T))
                ba.prepend(sizeof(T) - ba.size(),'\x00');
            return qFromBigEndian<T>(ba);
        }
    }
};

}

#endif // QX_BYTEARRAY_H
