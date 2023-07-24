#ifndef QX_APPLICATION_DESKTOP_ENTRY_H
#define QX_APPLICATION_DESKTOP_ENTRY_H

// Shared Lib Support
#include "qx/linux/qx_linux_export.h"

// Intra-component Includes
#include "qx/linux/qx-desktopentry.h"

namespace Qx
{

class QX_LINUX_EXPORT DesktopAction
{
//-Instance Members-------------------------------------------------------------------------------------------------
private:
    QString mActionName;
    QString mName;
    QString mIcon;
    QString mExec;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    DesktopAction();

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    QString actionName() const;
    QString name() const;
    QString icon() const;
    QString exec() const;

    void setActionName(const QString& name);
    void setName(const QString& name);
    void setIcon(const QString& icon);
    void setExec(const QString& exec);
};

class QX_LINUX_EXPORT ApplicationDesktopEntry : public DesktopEntry
{
//-Class Members-------------------------------------------------------------------------------------------------
private:
    static inline const QString TYPE = u"Application"_s;
    static inline const QString EXTENSION = u"desktop"_s;
    static inline const QString ACTION_HEADER = u"[Desktop Actions %1]"_s;

//-Instance Members-------------------------------------------------------------------------------------------------
private:
    bool mDBusActivatable;
    QString mTryExec;
    QString mExec;
    QString mPath;
    bool mTerminal;
    QHash<QString, DesktopAction> mActions;
    QStringList mMimeTypes;
    QStringList mCategories;
    QStringList mImplements;
    QStringList mKeywords;
    bool mStartupNotify;
    QString mStartupWMClass;
    bool mPrefersNonDefaultGPU;
    bool mSingleMainWindow;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    ApplicationDesktopEntry();

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    QString type() const override;
    QString extension() const override;
    QString toString() const override;

    bool isDBusActivatable();
    QString tryExec();
    QString exec();
    QString path();
    bool isTerminal();
    QList<DesktopAction> actions();
    DesktopAction action(const QString& actionName);
    QStringList mimeTypes() const;
    QStringList categories() const;
    QStringList implements() const;
    QStringList keywords() const;
    bool isStartupNotify() const;
    QString startupWMClass() const;
    bool isPrefersNonDefaultGPU() const;
    bool isSingleMainWindow() const;

    void setDBusActivatable(bool activatable);
    void setTryExec(const QString& tryExec);
    void setExec(const QString& exec);
    void setPath(const QString& path);
    void setTerminal(bool terminal);
    void insertAction(const DesktopAction& action);
    void removeAction(const QString& actionName);
    void setMimeTypes(const QStringList& mimeTypes);
    void setCategories(const QStringList& categories);
    void setImplements(const QStringList& implements);
    void setKeywords(const QStringList& keywords);
    void setStartupNotify(bool notify);
    void setStartupWMClass(const QString& wmClass);
    void setPrefersNonDefaultGPU(bool prefers);
    void setSingleMainWindow(bool single);
};

}

#endif // QX_APPLICATION_DESKTOP_ENTRY_H
