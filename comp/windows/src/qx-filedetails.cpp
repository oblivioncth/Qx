// Unit Includes
#include "qx/windows/qx-filedetails.h"

// Qt Includes
#include <QFileInfo>

namespace Qx
{

//===============================================================================================================
// FileDetails
//===============================================================================================================

/*!
 *  @class FileDetails qx/windows/qx-filedetails.h
 *  @ingroup qx-windows
 *
 *  @brief The FileDetails class acts as a user-friendly container for holding a file's @e Version Info as
 *  defined by the Windows API, which largely consists of the fields within a given file's @e Details pane.
 *
 *  File details are composed of two major sections:
 *  @li Fixed File Info - A fixed number of fields that are translation independent
 *  @li String Table - A set of optional fields that are translation dependent
 *
 *  The fixed portion of a file's details are accessible via the various named accessor functions of this class,
 *  while the string table portion is accessible via stringTable(). As the arguments for said function suggest,
 *  a file can contain multiple string tables, each representative of a specific translation, though most often
 *  files only contain one.
 *
 *  This class also features the static member function readFileDetails() that can be used to acquire the details
 *  of a given file.
 *
 *  @sa , <a href="https://docs.microsoft.com/en-us/windows/win32/menurc/versioninfo-resource">VERSIONINFO resource</a>
 *  <a href="https://docs.microsoft.com/en-us/windows/win32/api/winver/nf-winver-getfileversioninfow">GetFileVersionInfoW</a>,
 *  <a href="https://docs.microsoft.com/en-us/windows/win32/api/winver/nf-winver-verqueryvaluew">VerQueryValue</a>.
 */

//-Class Structs-----------------------------------------------------------------------------------------------------

//===============================================================================================================
// FileDetails::Translation
//===============================================================================================================
/*!
 *  @struct FileDetails::Translation qx/windows/qx-filedetails.h
 *
 *  @brief A structure used to represent a particular translation of the string table section of a file's details.
 *  It encapsulates a language and code page identifier pair.
 *
 *  @note Handling translations via code pages is deprecated and discouraged when created new Windows applications;
 *  however, file details are still encoded using this method for backwards compatibility, and therefore must be
 *  parsed the same way.
 */

/*!
 *  @var QString FileDetails::Translation::language
 *
 *  The language ID of the translation as a hexadecimal string.
 *
 *  @sa <a href="https://docs.microsoft.com/en-us/windows/win32/intl/language-identifiers">Language Identifiers</a>,
 *  <a href="https://docs.microsoft.com/en-us/windows/win32/menurc/versioninfo-resource#langIDs">VersionInfo Resource Language ID</a>.
 */

/*!
 *  @var QString FileDetails::Translation::codePage
 *
 *  The code page ID of the translation as a hexadecimal string.
 *
 *  @sa <a href="https://docs.microsoft.com/en-us/windows/win32/intl/code-page-identifiers">Code Page Identifiers</a>,
 *  <a href="https://docs.microsoft.com/en-us/windows/win32/menurc/versioninfo-resource#charsetID">VersionInfo Resource Code Page ID</a>.
 */

//-Operators----------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns @c true if the target and destination of @a lhs are the same as in @a rhs; otherwise returns @c false
 */
bool operator==(const FileDetails::Translation& lhs, const FileDetails::Translation& rhs) noexcept
{
    return lhs.language == rhs.language && lhs.codePage == rhs.codePage;
}

//-Hashing------------------------------------------------------------------------------------------------------
/*!
 *  Hashes the download task @a key with the initial @a seed.
 */
size_t qHash(const FileDetails::Translation& key, size_t seed) noexcept
{
    QtPrivate::QHashCombine hash;
    seed = hash(seed, key.language);
    seed = hash(seed, key.codePage);

    return seed;
}

//===============================================================================================================
// FileDetails::StringTable
//===============================================================================================================

/*!
 *  @struct FileDetails::StringTable qx/windows/qx-filedetails.h
 *
 *  @brief A structure that contains all of the translation dependent optional fields of a file's details
 *
 *  @sa <a href="https://docs.microsoft.com/en-us/windows/win32/menurc/stringtable">StringTable structure</a>.
 */

/*!
 *  @var QString FileDetails::StringTable::metaLanguageID
 *  The language identifier of the translation this string table is associated with.
 */

/*!
 *  @var QString FileDetails::StringTable::metaCodePageID
 *  The code page identifier of the translation this string table is associated with.
 */

/*!
 *  @var QString FileDetails::StringTable::comments
 *  Comments left by the file's author.
 */

/*!
 *  @var QString FileDetails::StringTable::companyName
 *  The company that authored the file.
 */

/*!
 *  @var QString FileDetails::StringTable::fileDescription
 *  A basic description of the file.
 */

/*!
 *  @var QString FileDetails::StringTable::fileVersion
 *  A string representation of the file's version.
 *
 *  Generally this is equivalent to:
 *  \code{.cpp}
 *  fileDetails.fileVersion().toString();
 *  \endcode
 */

/*!
 *  @var QString FileDetails::StringTable::internalName
 *  The internal name used to refer to the file by its author.
 */

/*!
 *  @var QString FileDetails::StringTable::legalCopyright
 *  Copyright information for the file.
 */

/*!
 *  @var QString FileDetails::StringTable::legalTrademarks
 *  Trademark information for the file.
 */

/*!
 *  @var QString FileDetails::StringTable::originalFilename
 *  The name of the file when it was authored.
 */

/*!
 *  @var QString FileDetails::StringTable::productName
 *  The name of the product this file is associated with.
 */

/*!
 *  @var QString FileDetails::StringTable::productVersion
 *  A string representation of the version of the product the file is associated with.
 *
 *  Generally this is equivalent to:
 *  \code{.cpp}
 *  fileDetails.productVersion().toString();
 *  \endcode
 */

/*!
 *  @var QString FileDetails::StringTable::privateBuild
 *  Information about the private version nature of the file. Only present if VS_FF_PRIVATEBUILD is set
 *  on this file.
 */

/*!
 *  @var QString FileDetails::StringTable::specialBuild
 *  Information about the special version nature of the file. Only present if VS_FF_SPECIALBUILD is set
 *  on this file.
 */

//-Constructor-------------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Creates a null FileDetails object.
 */
FileDetails::FileDetails() :
    mFileFlags(0),
    mFileOs(0),
    mFileType(0),
    mFileSubtype(0)
{}

//-Class Functions----------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns the file details of the file pointed to by @a filePath, or a null FileDetails object if
 *  the file doesn't exist.
 */
FileDetails FileDetails::readFileDetails(QString filePath)
{
    // File details to fill
    FileDetails workingFileDetails;

    if(QFileInfo::exists(filePath) && QFileInfo(filePath).isFile())
    {
        DWORD verInfoHandle, verInfoSize = GetFileVersionInfoSize((const wchar_t*)filePath.utf16(), &verInfoHandle);

        if (verInfoSize != NULL)
        {
            LPSTR verInfo = new char[verInfoSize];

            if (GetFileVersionInfo((const wchar_t*)filePath.utf16(), verInfoHandle, verInfoSize, verInfo))
            {
                LPBYTE viQueryResultBuffer = NULL;
                UINT viQuerryResultSizeBuffer = 0;

                // Read file and product versions
                if (VerQueryValue(verInfo, L"\\", (VOID FAR* FAR*)&viQueryResultBuffer, &viQuerryResultSizeBuffer))
                {
                    if (viQuerryResultSizeBuffer)
                    {
                        // Get fixed file info
                        VS_FIXEDFILEINFO* fixedFileInfo = (VS_FIXEDFILEINFO*)viQueryResultBuffer;
                        if (fixedFileInfo->dwSignature == 0xfeef04bd)
                        {
                            // Get struct version
                            workingFileDetails.mMetaStructVersion = VersionNumber(fixedFileInfo->dwStrucVersion >> 16,
                                                                                  fixedFileInfo->dwStrucVersion & 0xFFFF);

                            // Get file version
                            workingFileDetails.mFileVersion = VersionNumber(fixedFileInfo->dwFileVersionMS >> 16,
                                                                            fixedFileInfo->dwFileVersionMS & 0xFFFF,
                                                                            fixedFileInfo->dwFileVersionLS >> 16,
                                                                            fixedFileInfo->dwFileVersionLS & 0xFFFF);

                            // Get product version
                            workingFileDetails.mProductVersion = VersionNumber(fixedFileInfo->dwProductVersionMS >> 16,
                                                                               fixedFileInfo->dwProductVersionMS & 0xFFFF,
                                                                               fixedFileInfo->dwProductVersionLS >> 16,
                                                                               fixedFileInfo->dwProductVersionLS & 0xFFFF);

                            // Get file flags (use mask to validate bit fields)
                            workingFileDetails.mFileFlags = fixedFileInfo->dwFileFlags & fixedFileInfo->dwFileFlagsMask;

                            // Get target OSes
                            workingFileDetails.mFileOs = fixedFileInfo->dwFileOS;

                            // Get file type
                            workingFileDetails.mFileType = fixedFileInfo->dwFileType;

                            // Get file sub-type
                            workingFileDetails.mFileSubtype = fixedFileInfo->dwFileSubtype;

                            // TODO: DWORD dwFileDateMS and DWORD dwFileDateLS a currently unused
                        }
                    }
                }

                // Structure used to store enumerated languages and code pages.
                struct LANGANDCODEPAGE {
                    WORD wLanguage;
                    WORD wCodePage;
                } *langCodePage;

                // Lambda for repetitive string table queries
                std::function<QString(QString)> getStringTableVal = [&verInfo](QString query)->QString
                {
                    LPVOID lpPointer;
                    UINT bufferSize;

                    if(VerQueryValue(verInfo, (const wchar_t*)query.utf16(), &lpPointer, &bufferSize) && bufferSize)
                        return QString::fromStdWString((const TCHAR*)lpPointer);
                    else
                        return QString();

                };

                // Read the list of languages and code pages.
                VerQueryValue(verInfo, (const wchar_t*)FileDetails::LANG_CODE_PAGE_QUERY.utf16(), (LPVOID*)&langCodePage, &viQuerryResultSizeBuffer);

                // Read the file details for each language and code page.
                for (ULONGLONG i = 0; i < (viQuerryResultSizeBuffer / sizeof(struct LANGANDCODEPAGE)); i++)
                {
                    // Get current sub block base
                    QString langID = QString("%1").arg(langCodePage[i].wLanguage, 4, 16, QChar('0'));
                    QString codePageID = QString("%1").arg(langCodePage[i].wCodePage, 4, 16, QChar('0'));
                    QString currentSubBlockBase = FileDetails::SUB_BLOCK_BASE_TEMPLATE.arg(langID, codePageID);

                    // String table to populate
                    FileDetails::StringTable workingStringTable;
                    workingStringTable.metaLanguageID = langID.toUpper(); // Always capitalize the hex value
                    workingStringTable.metaCodePageID = codePageID.toUpper(); // Always capitalize the hex value

                    // Get current sub block values
                    workingStringTable.comments = getStringTableVal(currentSubBlockBase + FileDetails::ST_COMMENTS_QUERY);
                    workingStringTable.fileDescription = getStringTableVal(currentSubBlockBase + FileDetails::ST_FILE_DESCRIPTION_QUERY);
                    workingStringTable.fileVersion = getStringTableVal(currentSubBlockBase + FileDetails::ST_FILE_VERSION_QUERY);
                    workingStringTable.internalName = getStringTableVal(currentSubBlockBase + FileDetails::ST_INTERNAL_NAME_QUERY);
                    workingStringTable.legalCopyright = getStringTableVal(currentSubBlockBase + FileDetails::ST_LEGAL_COPYRIGHT_QUERY);
                    workingStringTable.legalTrademarks = getStringTableVal(currentSubBlockBase + FileDetails::ST_LEGAL_TRADEMARKS_QUERY);
                    workingStringTable.originalFilename = getStringTableVal(currentSubBlockBase + FileDetails::ST_ORIGINAL_FILENAME_QUERY);
                    workingStringTable.productName = getStringTableVal(currentSubBlockBase + FileDetails::ST_PRODUCT_NAME_QUERY);
                    workingStringTable.productVersion = getStringTableVal(currentSubBlockBase + FileDetails::ST_PRODUCT_VERSION_QUERY);
                    workingStringTable.privateBuild = getStringTableVal(currentSubBlockBase + FileDetails::ST_PRIVATE_BUILD_QUERY);
                    workingStringTable.specialBuild = getStringTableVal(currentSubBlockBase + FileDetails::ST_SPECIAL_BUILD_QUERY);

                    // Add string table to file details
                    workingFileDetails.addStringTable(workingStringTable);
                }
            }

            delete[] verInfo;
        }
    }

    // Return null or populated file details
    return workingFileDetails;
}

//-Instance Functions------------------------------------------------------------------------------------------------
//Private:
void FileDetails::addStringTable(StringTable stringTable)
{
    mStringTables.append(stringTable);
    mLangCodePageMap[Translation{stringTable.metaLanguageID, stringTable.metaCodePageID}] = mStringTables.count() - 1;
}

//Public:
/*!
 *  Returns @c true if the file details are null; otherwise returns @c false.
 */
bool FileDetails::isNull() { return mFileVersion.isNull() && mProductVersion.isNull() && mStringTables.isEmpty(); }

/*!
 *  Returns the number of string tables present within the file details.
 */
int FileDetails::stringTableCount() { return mStringTables.count(); }

/*!
 *  Returns the translations for which there are string tables available.
 */
QList<FileDetails::Translation> FileDetails::availableTranslations() { return mLangCodePageMap.keys(); }

/*!
 *  Returns @c true if a string table with the given @a translation is available; otherwise returns @c false.
 */
bool FileDetails::hasTranslation(Translation translation) { return mLangCodePageMap.contains(translation); }

/*!
 *  Returns the internal version of the VS_FIXEDFILEINFO structure that was read from the file.
 */
VersionNumber FileDetails::metaStructVersion() { return mMetaStructVersion; }

/*!
 *  Returns the version of the file.
 */
VersionNumber FileDetails::fileVersion() { return mFileVersion; }

/*!
 *  Returns the version of the product the file is associated with.
 */
VersionNumber FileDetails::productVersion() { return mProductVersion; }

/*!
 *  Returns the flags set on the file.
 *
 *  See the @e dwFileFlags section of the
 *  <a href="https://docs.microsoft.com/en-us/windows/win32/api/verrsrc/ns-verrsrc-vs_fixedfileinfo#members">
 *  VS_FIXEDFILEINFO</a> documentation for this values associated macros and their descriptions.
 */
DWORD FileDetails::fileFlags() { return mFileFlags; }

/*!
 *  Returns the target operating system for the file.
 *
 *  See the @e dwFileOS section of the
 *  <a href="https://docs.microsoft.com/en-us/windows/win32/api/verrsrc/ns-verrsrc-vs_fixedfileinfo#members">
 *  VS_FIXEDFILEINFO</a> documentation for this values associated macros and their descriptions.
 */
DWORD FileDetails::fileOs() { return mFileOs; }

/*!
 *  Returns the type of file.
 *
 *  See the @e dwFileType section of the
 *  <a href="https://docs.microsoft.com/en-us/windows/win32/api/verrsrc/ns-verrsrc-vs_fixedfileinfo#members">
 *  VS_FIXEDFILEINFO</a> documentation for this values associated macros and their descriptions.
 */
DWORD FileDetails::fileType() { return mFileType; }

/*!
 *  Returns the subtype of file.
 *
 *  See the @e dwFileSubtype section of the
 *  <a href="https://docs.microsoft.com/en-us/windows/win32/api/verrsrc/ns-verrsrc-vs_fixedfileinfo#members">
 *  VS_FIXEDFILEINFO</a> documentation for this values associated macros and their descriptions.
 */
DWORD FileDetails::fileSubType() { return mFileSubtype; }

/*!
 *  Returns the string table at index @a index.
 *
 *  The order of string tables within a file's version info is simply the order in which they were added when
 *  the file was authored.
 */
const FileDetails::StringTable FileDetails::stringTable(int index)
{
    if(index >= 0 && index < mStringTables.count())
        return mStringTables.at(index);
    else
        return FileDetails::StringTable();
}

/*!
 *  Returns the string table associated with the given @a translation, or a string table with all null
 *  fields if one with that translation is not available.
 */
const FileDetails::StringTable FileDetails::stringTable(Translation translation)
{
    if(hasTranslation(translation))
        return mStringTables.at(mLangCodePageMap.value(translation));
    else
        return StringTable();
}

}
