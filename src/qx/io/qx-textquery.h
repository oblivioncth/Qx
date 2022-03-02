#ifndef QX_TEXTQUERY_H
#define QX_TEXTQUERY_H

#include <QString>

#include "io/qx-textpos.h"

namespace Qx
{
	
class TextQuery
{
//-Instance Variables------------------------------------------------------------------------------------------------
private:
    QString mString;
    Qt::CaseSensitivity mCaseSensitivity;
    TextPos mStartPos;
    int mHitsToSkip;
    int mHitLimit;
    bool mAllowSplit;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    TextQuery(const QString& string, Qt::CaseSensitivity cs = Qt::CaseSensitive);

//-Instance Functions------------------------------------------------------------------------------------------------
public:
    const QString& string() const;
    Qt::CaseSensitivity caseSensitivity() const;
    TextPos startPosition() const;
    int hitsToSkip() const;
    int hitLimit() const;
    bool allowSplit() const;

    void setString(QString string);
    void setCaseSensitivity(Qt::CaseSensitivity caseSensitivity);
    void setStartPosition(TextPos startPosition);
    void setHitsToSkip(int hitsToSkip);
    void setHitLimit(int hitLimit);
    void setAllowSplit(bool allowSplit);
};

}

#endif // QX_TEXTQUERY_H