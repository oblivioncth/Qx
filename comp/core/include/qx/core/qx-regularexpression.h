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
    static inline const QRegularExpression HEX_ONLY =  QRegularExpression("^[a-fA-F0-9]+$", QRegularExpression::CaseInsensitiveOption);
    static inline const QRegularExpression ANY_NON_HEX = QRegularExpression("[^a-fA-F0-9 -]", QRegularExpression::CaseInsensitiveOption);
    static inline const QRegularExpression NUMBERS_ONLY = QRegularExpression("^[0-9]*$", QRegularExpression::CaseInsensitiveOption); // a digit (\d)
    static inline const QRegularExpression ALPHANUMERIC_ONLY = QRegularExpression("^[a-zA-Z0-9]*$", QRegularExpression::CaseInsensitiveOption);
    static inline const QRegularExpression LETTERS_ONLY = QRegularExpression("^[a-zA-Z]+$", QRegularExpression::CaseInsensitiveOption);
    static inline const QRegularExpression SEMANTIC_VERSION = QRegularExpression(
                "(?P<major>0|[1-9]\\d*)"
                "\\."
                "(?P<minor>0|[1-9]\\d*)"
                "\\."
                "(?P<patch>0|[1-9]\\d*)"
                "(?:-(?P<prerelease>(?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\\.(?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?"
                "(?:\\+(?P<buildmetadata>[0-9a-zA-Z-]+(?:\\.[0-9a-zA-Z-]+)*))?");
    static inline const QRegularExpression LONG_SEMANTIC_VERSION = QRegularExpression(
                "(?P<major>0|[1-9]\\d*)"
                "\\."
                "(?P<minor>0|[1-9]\\d*)"
                "\\."
                "(?P<patch>0|[1-9]\\d*)"
                "\\."
                "(?P<revision>0|[1-9]\\d*)"
                "(?:-(?P<prerelease>(?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\\.(?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?"
                "(?:\\+(?P<buildmetadata>[0-9a-zA-Z-]+(?:\\.[0-9a-zA-Z-]+)*))?");
};

}

#endif // QX_REGULAREXPRESSION_H
