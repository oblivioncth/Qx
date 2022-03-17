#ifndef QX_REGULAREXPRESSION_H
#define QX_REGULAREXPRESSION_H

// Qt Includes
#include <QRegularExpression>

namespace Qx
{
	
class RegularExpression
{
//-Class Variables---------------------------------------------------------------------------------------------
public:
    static inline const QRegularExpression HEX_ONLY =  QRegularExpression("^[0-9A-F]+$", QRegularExpression::CaseInsensitiveOption);
    static inline const QRegularExpression ANY_NON_HEX = QRegularExpression("[^a-fA-F0-9 -]", QRegularExpression::CaseInsensitiveOption);
    static inline const QRegularExpression NUMBERS_ONLY = QRegularExpression("^[0-9]*$", QRegularExpression::CaseInsensitiveOption); // a digit (\d)
    static inline const QRegularExpression ALPHANUMERIC_ONLY = QRegularExpression("^[a-zA-Z0-9]*$", QRegularExpression::CaseInsensitiveOption);
    static inline const QRegularExpression LETTERS_ONLY = QRegularExpression("^[a-zA-Z]+$", QRegularExpression::CaseInsensitiveOption);
};

}

#endif // QX_REGULAREXPRESSION_H
