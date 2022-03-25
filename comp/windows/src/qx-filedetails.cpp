// Unit Includes
#include "qx/windows/qx-filedetails.h"

// Qt Includes
#include <QFileInfo>

namespace Qx
{

//===============================================================================================================
// FileDetails
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------------
//Public:
FileDetails::FileDetails() :
    mFileFlags(0),
    mFileOs(0),
    mFileType(0),
    mFileSubtype(0)
{}

//-Class Functions----------------------------------------------------------------------------------------------------
//Public:
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
bool FileDetails::isNull() { return mFileVersion.isNull() && mProductVersion.isNull() && mStringTables.isEmpty(); }
int FileDetails::stringTableCount() { return mStringTables.count(); }
QList<FileDetails::Translation> FileDetails::availableTranslations() { return mLangCodePageMap.keys(); }
bool FileDetails::hasTranslation(Translation translation) { return mLangCodePageMap.contains(translation); }
VersionNumber FileDetails::metaStructVersion() { return mMetaStructVersion; }

VersionNumber FileDetails::fileVersion() { return mFileVersion; }
VersionNumber FileDetails::productVersion() { return mProductVersion; }
DWORD FileDetails::fileFlags() { return mFileFlags; }
DWORD FileDetails::targetSystems() { return mFileOs; }
DWORD FileDetails::fileType() { return mFileType; }
DWORD FileDetails::fileSubType() { return mFileSubtype; }

const FileDetails::StringTable FileDetails::stringTable(int index)
{
    if(index >= 0 && index < mStringTables.count())
        return mStringTables.at(index);
    else
        return FileDetails::StringTable();
}

const FileDetails::StringTable FileDetails::stringTable(Translation translation)
{
    if(hasTranslation(translation))
        return mStringTables.at(mLangCodePageMap.value(translation));
    else
        return StringTable();
}



}
