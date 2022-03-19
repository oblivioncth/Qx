// Unit Includes
#include "qx/io/qx-textpos.h"

namespace Qx
{
	
//===============================================================================================================
// TextPos
//===============================================================================================================

/*!
 *  @class TextPos
 *
 *  @brief The TextPos class is used to represent an offset within a text file in terms of lines and characters.
 */

//-Class Variables-----------------------------------------------------------------------------------------------
//Public:

/*!
 *  A text position representing the start of a file.
 *
 *  Equivalent to @c TextPos(0,0).
 */
const TextPos TextPos::START = TextPos(0,0); // Initialization of constant reference TextPos

/*!
 *  A text position representing the end file.
 *
 *  Equivalent to @c TextPos(Index32::LAST,Index32::LAST).
 */
const TextPos TextPos::END = TextPos(Index32::LAST, Index32::LAST); // Initialization of constant reference TextPos

//-Constructor---------------------------------------------------------------------------------------------------
//Public:

/*!
 *  Creates a null text position.
 */
TextPos::TextPos() :
    mLine(Index32()),
    mCharacter(Index32())
{}

/*!
 *  Creates a text position that points to @a line and @a character.
 */
TextPos::TextPos(Index32 line, Index32 character) :
    mLine(line),
    mCharacter(character)
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:

/*!
 *  Returns @c true if the line and character of this text position are the same as in @a otherTextPos;
 *  otherwise returns @c false.
 */
bool TextPos::operator==(const TextPos& otherTextPos) { return mLine == otherTextPos.mLine && mCharacter == otherTextPos.mCharacter; }

/*!
 *  Returns @c true if either the line or character of this text position is different than in @a otherTextPos;
 *  otherwise returns @c false.
 */
bool TextPos::operator!= (const TextPos& otherTextPos) { return !(*this == otherTextPos); }

/*!
 *  Returns @c true if this text position points to a further location than @a otherTextPos;
 *  otherwise returns @c false.
 */
bool TextPos::operator> (const TextPos& otherTextPos)
{
    if(mLine == otherTextPos.mLine)
        return mCharacter > otherTextPos.mCharacter;
    else
        return mLine > otherTextPos.mLine;
}

/*!
 *  Returns @c true if this text position points to at least the same location as @a otherTextPos;
 *  otherwise returns @c false.
 */
bool TextPos::operator>= (const TextPos& otherTextPos) { return *this == otherTextPos || *this > otherTextPos; }

/*!
 *  Returns @c true if this text position points to a closer location than @a otherTextPos;
 *  otherwise returns @c false.
 */
bool TextPos::operator< (const TextPos& otherTextPos) { return !(*this >= otherTextPos); }

/*!
 *  Returns @c true if this text position points to at most the same location as @a otherTextPos;
 *  otherwise returns @c false.
 */
bool TextPos::operator<= (const TextPos& otherTextPos) { return !(*this > otherTextPos); }

/*!
 *  Returns the line that the text position is pointing to.
 */
Index32 TextPos::line() const { return mLine; }

/*!
 *  Returns the character that the text position is pointing to.
 */
Index32 TextPos::character() const { return mCharacter; }

/*!
 *  Sets the text position to point to @a line.
 */
void TextPos::setLine(Index32 line) { mLine = line; }

/*!
 *  Sets the text position to point to @a character.
 */
void TextPos::setCharacter(Index32 character) { mCharacter = character; }

/*!
 *  Returns @c true if the text position is null; otherwise returns @c false.
 */
bool TextPos::isNull() const { return mLine.isNull() || mCharacter.isNull(); }

}
