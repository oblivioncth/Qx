#ifndef QX_DESKTOP_ENTRY_H
#define QX_DESKTOP_ENTRY_H

// Qt Includes
#include <QString>
#include <QUrl>

// Extra-component Includes
#include "qx/io/qx-ioopreport.h"

namespace Qx
{

/* TODO: Don't use values that were not explicitly set by the user when writing or converting to a string.
 * This is already the case for strings/string lists, can be enabled by an internal/public std::optional for
 * booleans.
 */

class DesktopEntry
{
//-Class Members----------------------------------------------------------------------------------------------------
private:
    static inline const QString MAIN_GROUP = QStringLiteral("[Desktop Entry]");

//-Instance Members-------------------------------------------------------------------------------------------------
private:
    QString mName;
    QString mGenericName;
    bool mNoDisplay;
    QString mComment;
    QString mIcon;
    bool mHidden;
    QStringList mOnlyShowIn;
    QStringList mNotShowIn;

//-Constructor-------------------------------------------------------------------------------------------------------
protected:
    DesktopEntry();

//-Destructor-------------------------------------------------------------------------------------------------------
public:
    virtual ~DesktopEntry() = default;

//-Class Functions----------------------------------------------------------------------------------------------
protected:
    static QString keyValueString(const QString& key, bool value);
    static QString keyValueString(const QString& key, const QString& value);
    static QString keyValueString(const QString& key, const QStringList& value);

public:
    static IoOpReport writeToDisk(QString path, const DesktopEntry* entry);

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    virtual QString type() const = 0;
    virtual QString extension() const = 0;
    virtual QString toString() const;

    QString name() const;
    QString genericName() const;
    bool isNoDisplay() const;
    QString comment() const;
    QString icon() const;
    bool isHidden() const;
    QStringList onlyShowIn() const;
    QStringList notShowIn() const;

    void setName(const QString& name);
    void setGenericName(const QString& name);
    void setNoDisplay(bool display);
    void setComment(const QString& comment);
    void setIcon(const QString& icon);
    void setHidden(bool hidden);
    void setOnlyShowIn(const QStringList& showIn);
    void setNotShowIn(const QStringList& notIn);
};

}

#endif // QX_DESKTOP_ENTRY_H
