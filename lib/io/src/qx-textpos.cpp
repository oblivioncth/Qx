// Unit Includes
#include "qx/io/qx-textpos.h"

namespace Qx
{
	
//===============================================================================================================
// TextPos
//===============================================================================================================

/*!
 *  @class TextPos qx/io/qx-textpos.h
 *  @ingroup qx-io
 *
 *  @brief The TextPos class is used to represent an offset within a text file in terms of lines and characters.
 */

//-Class Types-----------------------------------------------------------------------------------------------
//Public:

/*!
 *  @enum TextPos::Extent
 *
 *  Used to refer to text positions of special significance.
 *
 *  @var Index<T>::Extent Index<T>::First
 *  A text position representing the start of a file.
 *
 *  Equivalent to @c TextPos(0,0).
 *
 *  @var Index<T>::Extent Index<T>::Last
 *  A text position representing the end file.
 *
 *  Equivalent to @c TextPos(Index32(Index32::Last), Index32(Index32::Last)).
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:

/*!
 *  Creates a null text position.
 */
TextPos::TextPos() :
    mLine(Index32()),
    mCharacter(Index32())
{}

//TODO: Consider creating Qx::Extent (with Start/End) in qx-global and just use
// that for Index and TextPos.
/*!
 *  Creates a text position at the given extent @a e.
 */
TextPos::TextPos(Extent e)
{
    switch(e)
    {
        case Start:
            mLine = 0;
            mCharacter = 0;
            break;

        case End:
            mLine = Index32(Index32::Last);
            mCharacter = Index32(Index32::Last);
            break;

        default:
            qCritical("Invalid extent");
    }
}

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
 *  @fn bool TextPos::operator==(const TextPos& otherTextPos) const
 *
 *  Returns @c true if the line and character of this text position are the same as in @a otherTextPos;
 *  otherwise returns @c false.
 */

/*!
 *  Performs a three-way comparison between this text position and @a other.
 *
 *  Returns:
 *  - @c <0 if this text position is less than @a other
 *  - @c 0 if this text position is equal to @a other
 *  - @c >0 if this text position is greater than @a other
 */
auto TextPos::operator<=>(const TextPos& other) const noexcept
{
    if (auto c = mLine <=> other.mLine; c != 0)
            return c;

    return mCharacter <=> other.mCharacter;
}

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
