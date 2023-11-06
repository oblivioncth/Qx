#ifndef QX_FILEDETAILS_H
#define QX_FILEDETAILS_H

// Shared Lib Support
#include "qx/windows/qx_windows_export.h"

// Qt Includes
#include <QString>
#include <QHash>
#include <QDateTime>

// Intra-component Includes
#include "qx/windows/qx-windefs.h"

// Extra-component Includes
#include "qx/core/qx-versionnumber.h"

using namespace Qt::Literals::StringLiterals;

namespace Qx
{

class QX_WINDOWS_EXPORT FileDetails
{
//-Class Structs---------------------------------------------------------------------------------------------------------
public:
    struct Translation
    {
        QString language;
        QString codePage;

        friend QX_WINDOWS_EXPORT bool operator== (const Translation& lhs, const Translation& rhs) noexcept;
        friend QX_WINDOWS_EXPORT size_t qHash(const Translation& key, size_t seed) noexcept;
    };

    struct StringTable
    {
        QString metaLanguageID;
        QString metaCodePageID;
        QString comments;
        QString companyName;
        QString fileDescription;
        QString fileVersion;
        QString internalName;
        QString legalCopyright;
        QString legalTrademarks;
        QString originalFilename;
        QString productName;
        QString productVersion;
        QString privateBuild;
        QString specialBuild;
    };

//-Class Members----------------------------------------------------------------------------------------------------
private:
    static inline const QString LANG_CODE_PAGE_QUERY = u"\\VarFileInfo\\Translation"_s;
    static inline const QString SUB_BLOCK_BASE_TEMPLATE = u"\\StringFileInfo\\%1%2\\"_s;
    static inline const QString ST_COMMENTS_QUERY = u"Comments"_s;
    static inline const QString ST_COMPANY_NAME_QUERY = u"CompanyName"_s;
    static inline const QString ST_FILE_DESCRIPTION_QUERY = u"FileDescription"_s;
    static inline const QString ST_FILE_VERSION_QUERY = u"FileVersion"_s;
    static inline const QString ST_INTERNAL_NAME_QUERY = u"InternalName"_s;
    static inline const QString ST_LEGAL_COPYRIGHT_QUERY = u"LegalCopyright"_s;
    static inline const QString ST_LEGAL_TRADEMARKS_QUERY = u"LegalTrademarks"_s;
    static inline const QString ST_ORIGINAL_FILENAME_QUERY = u"OriginalFilename"_s;
    static inline const QString ST_PRODUCT_NAME_QUERY = u"ProductName"_s;
    static inline const QString ST_PRODUCT_VERSION_QUERY = u"ProductVersion"_s;
    static inline const QString ST_PRIVATE_BUILD_QUERY = u"PrivateBuild"_s;
    static inline const QString ST_SPECIAL_BUILD_QUERY = u"SpecialBuild"_s;

//-Instance Members-------------------------------------------------------------------------------------------------
private:
    VersionNumber mMetaStructVersion;
    VersionNumber mFileVersion;
    VersionNumber mProductVersion;
    DWORD mFileFlags;
    DWORD mFileOs;
    DWORD mFileType;
    DWORD mFileSubtype;
    QDateTime mFileDate;
    QList<StringTable> mStringTables;
    QHash<Translation, int> mLangCodePageMap;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    FileDetails();

//-Class Functions-------------------------------------------------------------------------------------------------
public:
    static FileDetails readFileDetails(QString filePath);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void addStringTable(StringTable stringTable);

public:
    bool isNull();
    int stringTableCount();
    QList<Translation> availableTranslations();
    bool hasTranslation(Translation translation);
    VersionNumber metaStructVersion();

    VersionNumber fileVersion();
    VersionNumber productVersion();
    DWORD fileFlags();
    DWORD fileOs();
    DWORD fileType();
    DWORD fileSubType();
    QDateTime fileDate();
    const StringTable stringTable(int index = 0);
    const StringTable stringTable(Translation translation);
};

}

#endif // QX_FILEDETAILS_H
