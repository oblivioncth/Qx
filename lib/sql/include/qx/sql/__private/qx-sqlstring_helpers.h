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

void inline appendKeyword(QString& str, const QString& word) { append(str, word); }

}
/*! @endcond */

#endif // QX_SQLSTRING_HELPERS_H
