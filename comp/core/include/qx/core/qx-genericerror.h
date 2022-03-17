#ifndef QX_GENERICERROR_H
#define QX_GENERICERROR_H

// Qt Includes
#include <QHash>
#include <QString>
#include <QMetaType>
#include <QTextStream>

namespace Qx
{

class GenericError
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

    GenericError& setErrorLevel(ErrorLevel errorLevel);
    GenericError& setCaption(QString caption);
    GenericError& setPrimaryInfo(QString primaryInfo);
    GenericError& setSecondaryInfo(QString secondaryInfo);
    GenericError& setDetailedInfo(QString detailedInfo);

//-Friend Functions------------------------------------------------------------------------------------------------
friend QTextStream& operator<<(QTextStream& ts, const GenericError& ge);
};

//-Non-member/Related Functions------------------------------------------------------------------------------------
QTextStream& operator<<(QTextStream& ts, const GenericError& ge);

}

//-Metatype declarations-------------------------------------------------------------------------------------------
Q_DECLARE_METATYPE(Qx::GenericError);

#endif // QX_GENERICERROR_H
