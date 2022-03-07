// Unit Includes
#include "qx/io/qx-textpos.h"

namespace Qx
{
	
//===============================================================================================================
// TextPos
//===============================================================================================================

//-Class Variables-----------------------------------------------------------------------------------------------
//Public:
const TextPos TextPos::START = TextPos(0,0); // Initialization of constant reference TextPos
const TextPos TextPos::END = TextPos(Index32::LAST, Index32::LAST); // Initialization of constant reference TextPos

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
TextPos::TextPos() :
    mLine(Index32()),
    mCharacter(Index32())
{}

TextPos::TextPos(Index32 line, Index32 character) :
    mLine(line),
    mCharacter(character)
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
bool TextPos::operator==(const TextPos& otherTextPos) { return mLine == otherTextPos.mLine && mCharacter == otherTextPos.mCharacter; }
bool TextPos::operator!= (const TextPos& otherTextPos) { return !(*this == otherTextPos); }
bool TextPos::operator> (const TextPos& otherTextPos)
{
    if(mLine == otherTextPos.mLine)
        return mCharacter > otherTextPos.mCharacter;
    else
        return mLine > otherTextPos.mLine;
}
bool TextPos::operator>= (const TextPos& otherTextPos) { return *this == otherTextPos || *this > otherTextPos; }
bool TextPos::operator< (const TextPos& otherTextPos) { return !(*this >= otherTextPos); }
bool TextPos::operator<= (const TextPos& otherTextPos) { return !(*this > otherTextPos); }

Index32 TextPos::line() const { return mLine; }
Index32 TextPos::character() const { return mCharacter; }
void TextPos::setLine(Index32 line) { mLine = line; }
void TextPos::setCharacter(Index32 character) { mCharacter = character; }
bool TextPos::isNull() const { return mLine.isNull() || mCharacter.isNull(); }

}
