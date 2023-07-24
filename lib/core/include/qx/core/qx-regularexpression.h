#ifndef QX_REGULAREXPRESSION_H
#define QX_REGULAREXPRESSION_H

// Qt Includes
#include <QRegularExpression>

using namespace Qt::Literals::StringLiterals;

namespace Qx
{
	
class RegularExpression
{
//-Class Variables---------------------------------------------------------------------------------------------
public:
    static inline const QRegularExpression HEX_ONLY =  QRegularExpression(u"^[a-fA-F0-9]+$"_s, QRegularExpression::CaseInsensitiveOption);
    static inline const QRegularExpression ANY_NON_HEX = QRegularExpression(u"[^a-fA-F0-9 -]"_s, QRegularExpression::CaseInsensitiveOption);
    static inline const QRegularExpression NUMBERS_ONLY = QRegularExpression(u"^[0-9]*$"_s, QRegularExpression::CaseInsensitiveOption); // a digit (\d)
    static inline const QRegularExpression ALPHANUMERIC_ONLY = QRegularExpression(u"^[a-zA-Z0-9]*$"_s, QRegularExpression::CaseInsensitiveOption);
    static inline const QRegularExpression LETTERS_ONLY = QRegularExpression(u"^[a-zA-Z]+$"_s, QRegularExpression::CaseInsensitiveOption);
    static inline const QRegularExpression SEMANTIC_VERSION = QRegularExpression(
                u"(?P<major>0|[1-9]\\d*)"_s
                u"\\."_s
                u"(?P<minor>0|[1-9]\\d*)"_s
                u"\\."_s
                u"(?P<patch>0|[1-9]\\d*)"_s
                u"(?:-(?P<prerelease>(?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\\.(?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?"_s
        u"(?:\\+(?P<buildmetadata>[0-9a-zA-Z-]+(?:\\.[0-9a-zA-Z-]+)*))?"_s);
    static inline const QRegularExpression LONG_SEMANTIC_VERSION = QRegularExpression(
                u"(?P<major>0|[1-9]\\d*)"_s
                u"\\."_s
                u"(?P<minor>0|[1-9]\\d*)"_s
                u"\\."_s
                u"(?P<patch>0|[1-9]\\d*)"_s
                u"\\."_s
                u"(?P<revision>0|[1-9]\\d*)"_s
                u"(?:-(?P<prerelease>(?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\\.(?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?"_s
                u"(?:\\+(?P<buildmetadata>[0-9a-zA-Z-]+(?:\\.[0-9a-zA-Z-]+)*))?"_s);
    static inline const QRegularExpression LINE_BREAKS = QRegularExpression(u"[\\r\\n\\v]"_s);
    static inline const QRegularExpression WHITESPACE = QRegularExpression(u"[\\f\\n\\r\\t\\v\x20\xA0\x00A0\u2028\u2029]"_s);
};

}

#endif // QX_REGULAREXPRESSION_H
