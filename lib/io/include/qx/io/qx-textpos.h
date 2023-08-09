#ifndef QX_TEXTPOS_H
#define QX_TEXTPOS_H

// Shared Lib Support
#include "qx/io/qx_io_export.h"

// Extra-component Includes
#include "qx/core/qx-index.h"

// TODO: This class should probably be constexpr construable, though this
// would mean it cant have separate implementation files which is somewhat ugly.
// probably should reconsider this once using modules.

namespace Qx
{

class QX_IO_EXPORT TextPos
{
//-Class Types------------------------------------------------------------------------------------------------------
public:
    enum Extent
    {
        Start,
        End
    };

//-Instance Variables------------------------------------------------------------------------------------------------
private:
    Index32 mLine;
    Index32 mCharacter;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    TextPos();
    TextPos(Extent e);
    TextPos(Index32 line, Index32 character);

//-Instance Functions------------------------------------------------------------------------------------------------
public:
    Index32 line() const;
    Index32 character() const;
    void setLine(Index32 line);
    void setCharacter(Index32 character);
    bool isNull() const;

    bool operator==(const TextPos& other) const noexcept = default;
    std::strong_ordering operator<=>(const TextPos& other) const noexcept;
};

}

#endif // QX_TEXTPOS_H
