// Unit Includes
#include "qx/io/qx-textquery.h"

namespace Qx
{
	
//===============================================================================================================
// TextQuery
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
TextQuery::TextQuery(const QString& string, Qt::CaseSensitivity cs) :
    mString(string),
    mCaseSensitivity(cs),
    mStartPos(TextPos::START),
    mHitsToSkip(0),
    mHitLimit(-1),
    mAllowSplit(false)
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
const QString& TextQuery::string() const { return mString; }
Qt::CaseSensitivity TextQuery::caseSensitivity() const { return mCaseSensitivity; }
TextPos TextQuery::startPosition() const { return mStartPos; }
int TextQuery::hitsToSkip() const { return mHitsToSkip; }
int TextQuery::hitLimit() const { return mHitLimit; }
bool TextQuery::allowSplit() const{ return mAllowSplit; }

void TextQuery::setString(QString string) { mString = string; }
void TextQuery::setCaseSensitivity(Qt::CaseSensitivity caseSensitivity) { mCaseSensitivity = caseSensitivity; }
void TextQuery::setStartPosition(TextPos startPosition) { mStartPos = startPosition; }
void TextQuery::setHitsToSkip(int hitsToSkip) { mHitsToSkip = std::min(hitsToSkip, 0); }
void TextQuery::setHitLimit(int hitLimit) { mHitLimit = hitLimit; }
void TextQuery::setAllowSplit(bool allowSplit) { mAllowSplit = allowSplit; }

}
