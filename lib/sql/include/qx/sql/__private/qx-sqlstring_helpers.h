#ifndef QX_SQLSTRING_HELPERS_H
#define QX_SQLSTRING_HELPERS_H

// Qt Includes
#include <QStringView>
#include <QString>

// Intra-component Includes
#include <qx/sql/qx-sqlstring.h>

/*! @cond */
namespace _QxPrivate
{

namespace __private
{
    template<typename S>
        requires std::constructible_from<QString, S>
    decltype(auto) qstring_compat(S&& s) { return std::forward<S>(s); }
    inline QString qstring_compat(const Qx::SqlString& s) { return s.toString(); }

    template<typename First, typename ...Rest>
    QString join(First&& first, Rest&&... rest)
    {
        QString result = qstring_compat(first);
        ((result += "," + qstring_compat(rest)), ...);
        return result;
    }
}

void inline append(QString& str, QStringView sql, bool space = true)
{
    using namespace Qt::StringLiterals;
    if(!str.isEmpty() && space)
        str += u" "_s;

    str += sql;
}
template<typename FirstArg, typename ...RestArgs>
void appendKeyword(QString& str, const QString& word, FirstArg&& firstArg, RestArgs&&... restArgs)
{
    append(str, word);
    append(str, __private::join(std::forward<FirstArg>(firstArg), std::forward<RestArgs>(restArgs)...));
}

template<typename FirstArg, typename ...RestArgs>
void appendKeywordParen(QString& str, const QString& word, FirstArg&& firstArg, RestArgs&&... restArgs)
{
    append(str, word);
    append(str, u"("_s + __private::join(std::forward<FirstArg>(firstArg), std::forward<RestArgs>(restArgs)...) + u")"_s);
}

template <std::ranges::input_range R>
void appendKeywordParen(QString& str, const QString& word, const R& range)
{
    /* The boxing here is inefficient, but I'm not sure how to improve the situation since
     * we rely on the SqlString ctor to reliably get a string from value_type.
     */
    QString rStr = u"'"_s;
    for(auto n = std::size(range); const auto& value : range)
    {
        rStr += __private::qstring_compat(value);
        if(n-- != 1)
            rStr += u"','"_s;
    }
    rStr += u"'"_s;

    appendKeywordParen(str, word, rStr);
}

void inline appendKeyword(QString& str, const QString& word) { append(str, word); }

}
/*! @endcond */

#endif // QX_SQLSTRING_HELPERS_H
