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
    static inline const QRegularExpression hexOnly =  QRegularExpression("^[0-9A-F]+$", QRegularExpression::CaseInsensitiveOption);
    static inline const QRegularExpression anyNonHex = QRegularExpression("[^a-fA-F0-9 -]", QRegularExpression::CaseInsensitiveOption);
    static inline const QRegularExpression numbersOnly = QRegularExpression("^[0-9]*$", QRegularExpression::CaseInsensitiveOption); // a digit (\d)
    static inline const QRegularExpression alphanumericOnly = QRegularExpression("^[a-zA-Z0-9]*$", QRegularExpression::CaseInsensitiveOption);
    static inline const QRegularExpression lettersOnly = QRegularExpression("^[a-zA-Z]+$", QRegularExpression::CaseInsensitiveOption);
};

}

#endif // QX_REGULAREXPRESSION_H
