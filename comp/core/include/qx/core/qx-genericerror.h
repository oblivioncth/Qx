#ifndef QX_GENERICERROR_H
#define QX_GENERICERROR_H

// Qt Includes
#include <QHash>
#include <QString>
#include <QMetaType>

#ifdef QT_WIDGETS_LIB // Only enabled for Widgets edition
    #include <QMessageBox>
#endif

namespace Qx
{

class GenericError //TODO - Have Qx functions that use this class return "default" error levels instead of undefined ones (document them once documentation starts
{
//-Class Enums-----------------------------------------------------------------------------------------------
public:
    enum ErrorLevel {Warning, Error, Critical };

//-Class Members---------------------------------------------------------------------------------------------
private:
    static inline const QHash<ErrorLevel, QString> ERR_LVL_STRING_MAP = {
        {ErrorLevel::Warning, "Warning"},
        {ErrorLevel::Error, "Error"},
        {ErrorLevel::Critical, "Critical"},
    };

    static inline const QString DETAILED_INFO_HEADING = "Details:\n--------";

public:
    static const GenericError UNKNOWN_ERROR;

//-Instance Members------------------------------------------------------------------------------------------
private:
    ErrorLevel mErrorLevel;
    QString mCaption;
    QString mPrimaryInfo;
    QString mSecondaryInfo;
    QString mDetailedInfo;

//-Constructor----------------------------------------------------------------------------------------------
public:
    GenericError();
    GenericError(ErrorLevel errorLevel, QString primaryInfo,
                 QString secondaryInfo = QString(), QString detailedInfo = QString(), QString caption = QString());

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    bool isValid() const;
    ErrorLevel errorLevel() const;
    QString errorLevelString(bool caps = true) const;
    QString caption() const;
    QString primaryInfo() const;
    QString secondaryInfo() const;
    QString detailedInfo() const;

    Qx::GenericError& setErrorLevel(ErrorLevel errorLevel);
    Qx::GenericError& setCaption(QString caption);
    Qx::GenericError& setPrimaryInfo(QString primaryInfo);
    Qx::GenericError& setSecondaryInfo(QString secondaryInfo);
    Qx::GenericError& setDetailedInfo(QString detailedInfo);

#ifdef QT_WIDGETS_LIB // Only enabled for Widgets edition
    int exec(QMessageBox::StandardButtons choices, QMessageBox::StandardButton defChoice = QMessageBox::NoButton) const;
#else
    void print() const;
#endif

};

}

//-Metatype declarations-------------------------------------------------------------------------------------------
Q_DECLARE_METATYPE(Qx::GenericError);

#endif // QX_GENERICERROR_H
