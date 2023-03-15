#ifndef QX_TEXTPOS_H
#define QX_TEXTPOS_H

// Shared Lib Support
#include "qx/io/qx_io_export.h"

// Extra-component Includes
#include "qx/core/qx-index.h"

namespace Qx
{

class QX_IO_EXPORT TextPos
{
//-Class Types------------------------------------------------------------------------------------------------------
public:
    static const TextPos START;
    static const TextPos END;

//-Instance Variables------------------------------------------------------------------------------------------------
private:
    Index32 mLine;
    Index32 mCharacter;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    TextPos();
    TextPos(Index32 line, Index32 character);

//-Instance Functions------------------------------------------------------------------------------------------------
public:
    Index32 line() const;
    Index32 character() const;
    void setLine(Index32 line);
    void setCharacter(Index32 character);
    bool isNull() const;

    bool operator== (const TextPos& otherTextPos);
    bool operator!= (const TextPos& otherTextPos);
    bool operator> (const TextPos& otherTextPos);
    bool operator>= (const TextPos& otherTextPos);
    bool operator< (const TextPos& otherTextPos);
    bool operator<= (const TextPos& otherTextPos);
};

}

#endif // QX_TEXTPOS_H
