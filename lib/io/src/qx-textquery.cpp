// Unit Includes
#include "qx/io/qx-textquery.h"

namespace Qx
{
	
//===============================================================================================================
// TextQuery
//===============================================================================================================

/*!
 *  @class TextQuery qx/io/qx-textquery.h
 *  @ingroup qx-io
 *
 *  @brief The TextQuery class contains rules for conducting a search of text.
 *
 *  @note It is up to the implementation of a class or function that uses a text query to abide by those rules.
 *
 *  @sa findStringInFile()
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Creates a text query with the target @a string, abiding by @a cs case sensitivity.
 */
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
/*!
 *  Returns the query's target string.
 */
const QString& TextQuery::string() const { return mString; }

/*!
 *  Returns the case sensitivity of the query.
 */
Qt::CaseSensitivity TextQuery::caseSensitivity() const { return mCaseSensitivity; }

/*!
 *  Returns the position from which the search should begin.
 *
 *  The default is @c TextPos::START.
 */
TextPos TextQuery::startPosition() const { return mStartPos; }

/*!
 *  Returns the number of times to skip the query string before a true match is made.
 *
 *  The default is 0.
 */
int TextQuery::hitsToSkip() const { return mHitsToSkip; }


/*!
 *  Returns the number of matches to make before stopping the search.
 *
 *  The default is -1 (unlimited).
 */
int TextQuery::hitLimit() const { return mHitLimit; }

/*!
 *  Returns whether or not the target string will be matched if it falls across one or more line breaks.
 *
 *  The default is @c false.
 */
bool TextQuery::allowSplit() const{ return mAllowSplit; }

/*!
 *  Sets the query's target string.
 */
void TextQuery::setString(QString string) { mString = string; }

/*!
 *  Sets the case sensitivity of the query.
 */
void TextQuery::setCaseSensitivity(Qt::CaseSensitivity caseSensitivity) { mCaseSensitivity = caseSensitivity; }

/*!
 *  Sets the position from which the search should begin.
 */
void TextQuery::setStartPosition(TextPos startPosition) { mStartPos = startPosition; }

/*!
 *  Sets the number of times to skip the query string before a true match is made.
 */
void TextQuery::setHitsToSkip(int hitsToSkip) { mHitsToSkip = std::min(hitsToSkip, 0); }

/*!
 *  Sets the the number of matches to make before stopping the search.
 *
 *  Use -1 for no limit.
 */
void TextQuery::setHitLimit(int hitLimit) { mHitLimit = hitLimit; }

/*!
 *  Sets whether or not the target string will be matched if it falls across one or more line breaks.
 */
void TextQuery::setAllowSplit(bool allowSplit) { mAllowSplit = allowSplit; }

}
