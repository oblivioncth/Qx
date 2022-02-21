#include "qx-char.h"

#include "core/qx-regularexpression.h"

namespace Qx
{
	
//===============================================================================================================
// Char
//===============================================================================================================

//-Class Functions-----------------------------------------------------------------------------------------------
//Public:
bool Char::isHexNumber(QChar hexNum) { return RegularExpression::hexOnly.match(hexNum).hasMatch(); }

bool Char::compare(QChar cOne, QChar cTwo, Qt::CaseSensitivity cs)
{
    return cOne == cTwo || (cs == Qt::CaseInsensitive && cOne.toLower() == cTwo.toLower());
}
	
}
