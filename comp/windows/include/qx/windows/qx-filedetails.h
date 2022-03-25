#ifndef QX_FILEDETAILS_H
#define QX_FILEDETAILS_H

// Qt Includes
#include <QString>
#include <QHash>

// Windows Includes
#define NOMINMAX
#include "Windows.h"
#undef NOMINMAX

// Extra-component Includes
#include "qx/core/qx-versionnumber.h"

namespace Qx
{

class FileDetails
{
//-Class Structs---------------------------------------------------------------------------------------------------------
public:
    struct Translation
    {
        QString language;
        QString codePage;
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
    static inline const QString LANG_CODE_PAGE_QUERY = "\\VarFileInfo\\Translation";
    static inline const QString SUB_BLOCK_BASE_TEMPLATE = "\\StringFileInfo\\%1%2\\";
    static inline const QString ST_COMMENTS_QUERY = "Comments";
    static inline const QString ST_COMPANY_NAME_QUERY = "CompanyName";
    static inline const QString ST_FILE_DESCRIPTION_QUERY = "FileDescription";
    static inline const QString ST_FILE_VERSION_QUERY = "FileVersion";
    static inline const QString ST_INTERNAL_NAME_QUERY = "InternalName";
    static inline const QString ST_LEGAL_COPYRIGHT_QUERY = "LegalCopyright";
    static inline const QString ST_LEGAL_TRADEMARKS_QUERY = "LegalTrademarks";
    static inline const QString ST_ORIGINAL_FILENAME_QUERY = "OriginalFilename";
    static inline const QString ST_PRODUCT_NAME_QUERY = "ProductName";
    static inline const QString ST_PRODUCT_VERSION_QUERY = "ProductVersion";
    static inline const QString ST_PRIVATE_BUILD_QUERY = "PrivateBuild";
    static inline const QString ST_SPECIAL_BUILD_QUERY = "SpecialBuild";

//-Instance Members-------------------------------------------------------------------------------------------------
private:
    VersionNumber mMetaStructVersion;
    VersionNumber mFileVersion;
    VersionNumber mProductVersion;
    DWORD mFileFlags;
    DWORD mFileOs;
    DWORD mFileType;
    DWORD mFileSubtype;
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
    const StringTable stringTable(int index = 0);
    const StringTable stringTable(Translation translation);
};

}

#endif // QX_FILEDETAILS_H
