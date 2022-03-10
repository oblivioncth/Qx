// Unit Includes
#include "qx/core/qx-char.h"

// Intra-component Includes
#include "qx/core/qx-regularexpression.h"

namespace Qx
{
	
//===============================================================================================================
// Char
//===============================================================================================================

//-Class Functions-----------------------------------------------------------------------------------------------
//Public:
bool Char::isHexNumber(QChar hexNum) { return RegularExpression::hexOnly.match(hexNum).hasMatch(); }

int Char::compare(QChar cOne, QChar cTwo, Qt::CaseSensitivity cs)
{
    // Equalize case if case-insensitive
    if(cs == Qt::CaseInsensitive)
    {
        cOne = cOne.toLower();
        cTwo = cTwo.toLower();
    }

    if(cOne < cTwo)
        return -1;
    else if(cOne > cTwo)
        return 1;
    else
        return 0;
}
	
}
