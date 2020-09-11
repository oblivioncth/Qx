#include "qx-windows.h"
#include "qx-io.h"
#include <tlhelp32.h>
#include <QFileInfo>
#include <QCoreApplication>

namespace Qx
{
//-Classes------------------------------------------------------------------------------------------------------------

//===============================================================================================================
// FILE DETAILS
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------------
//Public:
FileDetails::FileDetails() {}

//-Instance Functions------------------------------------------------------------------------------------------------
//Public:
void FileDetails::addStringTable(StringTable stringTable)
{
    mStringTables.append(stringTable);
    mLangCodePageMap[qMakePair(stringTable.metaLanguageID, stringTable.metaCodePageID)] = mStringTables.count() - 1;
}

//Private:
bool FileDetails::isNull() { return mFileVersion.isNull() && mProductVersion.isNull() && mStringTables.isEmpty(); }
int FileDetails::stringTableCount() { return mStringTables.count(); }
QList<QPair<QString, QString>> FileDetails::availableLangCodePages() { return mLangCodePageMap.keys(); }
bool FileDetails::hasLangCodePage(QString lanuage, QString codePage) { return mLangCodePageMap.contains(qMakePair(lanuage.toUpper(), codePage.toUpper())); }
MMRB FileDetails::metaStructVersion() { return mMetaStructVersion; }

MMRB FileDetails::getFileVersion() { return mFileVersion; }
MMRB FileDetails::getProductVersion() { return mProductVersion; }
FileDetails::FileFlags FileDetails::getFileFlags() { return mFileFlags; }
FileDetails::TargetSystems FileDetails::getTargetSystems() { return mTargetSystems; }
FileDetails::FileType FileDetails::getFileType() { return mFileType; }
FileDetails::FileSubType FileDetails::getFileSubType() { return mFileSubType; }
int FileDetails::getVirtualDeviceID() { return mVirtualDeviceID; }

const FileDetails::StringTable FileDetails::getStringTable(int index)
{
    if(index >= 0 && index < mStringTables.count())
        return mStringTables.at(index);
    else
        return FileDetails::StringTable();
}

const FileDetails::StringTable FileDetails::getStringTable(QString language, QString codePage)
{
    if(hasLangCodePage(language.toUpper(), codePage.toUpper()))
        return mStringTables.at(mLangCodePageMap.value(qMakePair(language.toUpper(), codePage.toUpper())));
    else
        return StringTable();
}

//-Functions-------------------------------------------------------------------------------------------------------------
FileDetails getFileDetails(QString filePath)
{
    // File details to fill
    FileDetails workingFileDetails;

    if(QFileInfo::exists(filePath) && QFileInfo(filePath).isFile())
    {
        DWORD verInfoHandle, verInfoSize = GetFileVersionInfoSize(filePath.toStdWString().c_str(), &verInfoHandle);

        if (verInfoSize != NULL)
        {
            LPSTR verInfo = new char[verInfoSize];

            if (GetFileVersionInfo(filePath.toStdWString().c_str(), verInfoHandle, verInfoSize, verInfo))
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
                            workingFileDetails.mMetaStructVersion = MMRB(fixedFileInfo->dwStrucVersion >> 16,
                                                                         fixedFileInfo->dwStrucVersion & 0xFFFF,
                                                                         0, 0);

                            // Get file version
                            workingFileDetails.mFileVersion = MMRB(fixedFileInfo->dwFileVersionMS >> 16,
                                                                   fixedFileInfo->dwFileVersionMS & 0xFFFF,
                                                                   fixedFileInfo->dwFileVersionLS >> 16,
                                                                   fixedFileInfo->dwFileVersionLS & 0xFFFF);

                            // Get product version
                            workingFileDetails.mProductVersion = MMRB(fixedFileInfo->dwProductVersionMS >> 16,
                                                                      fixedFileInfo->dwProductVersionMS & 0xFFFF,
                                                                      fixedFileInfo->dwProductVersionLS >> 16,
                                                                      fixedFileInfo->dwProductVersionLS & 0xFFFF);

                            // Get file flags
                            DWORD trueFlags = fixedFileInfo->dwFileFlags & fixedFileInfo->dwFileFlagsMask;
                            QHash<DWORD, FileDetails::FileFlag>::const_iterator i;
                            for (i = FileDetails::FILE_FLAG_MAP.constBegin(); i != FileDetails::FILE_FLAG_MAP.constEnd(); i++)
                                if(trueFlags & i.key())
                                    workingFileDetails.mFileFlags = workingFileDetails.mFileFlags | i.value();

                            // Get target OSes
                            QHash<DWORD, FileDetails::TargetSystem>::const_iterator j;
                            for (j = FileDetails::TARGET_SYSTEM_MAP.constBegin(); j != FileDetails::TARGET_SYSTEM_MAP.constEnd(); j++)
                                if(fixedFileInfo->dwFileOS & j.key())
                                    workingFileDetails.mTargetSystems = workingFileDetails.mTargetSystems | j.value();

                            // Get file type
                            workingFileDetails.mFileType = fixedFileInfo->dwFileType == 0 ? FileDetails::FT_NONE :
                                                           FileDetails::FILE_TYPE_MAP.value(fixedFileInfo->dwFileType, FileDetails::FT_UNK);

                            // Get file sub-type
                            workingFileDetails.mFileSubType = fixedFileInfo->dwFileSubtype == 0 ? FileDetails::FST_NONE :
                                                              FileDetails::FILE_SUB_TYPE_MAP.value(qMakePair(workingFileDetails.mFileType,
                                                                                                             fixedFileInfo->dwFileSubtype),
                                                                                                   FileDetails::FST_UNK);
                            // DWORD dwFileDateMS and DWORD dwFileDateLS a currently unused
                        }
                    }
                }

                // Structure used to store enumerated languages and code pages.
                struct LANGANDCODEPAGE {
                    WORD wLanguage;
                    WORD wCodePage;
                } *langCodePage;

                // Lambda for repetative string table queries
                std::function<QString(QString)> getStringTableVal = [&verInfo](QString query)->QString
                {
                    LPVOID lpPointer;
                    UINT bufferSize;

                    if(VerQueryValue(verInfo, query.toStdWString().c_str(), &lpPointer, &bufferSize) && bufferSize)
                        return QString::fromStdWString((const TCHAR*)lpPointer);
                    else
                        return QString();

                };

                // Read the list of languages and code pages.
                VerQueryValue(verInfo, FileDetails::LANG_CODE_PAGE_QUERY.toStdWString().c_str(), (LPVOID*)&langCodePage, &viQuerryResultSizeBuffer);

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

DWORD getProcessIDByName(QString processName)
{
    // Find process ID by name
    DWORD processID = 0;
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if (QString::fromWCharArray(entry.szExeFile) == processName)
            {
                processID = entry.th32ProcessID;
                break;
            }
        }
    }

    CloseHandle(snapshot);

    // Return if found or not (0 if not)
    return processID;
}

QString getProcessNameByID(DWORD processID)
{
    // Find process name by ID
    QString processName = QString();
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if (entry.th32ProcessID == processID)
            {
                processName = QString::fromWCharArray(entry.szExeFile);
                break;
            }
        }
    }

    CloseHandle(snapshot);

    // Return if found or not (Null if not)
    return processName;
}

bool processIsRunning(QString processName) { return getProcessIDByName(processName); }
bool processIsRunning(DWORD processID) { return getProcessNameByID(processID).isNull(); }

bool enforceSingleInstance()
{
    // Presistant handle instance
    static HANDLE uniqueAppMutex = NULL;

    // Get self hash
    QFile selfEXE(QCoreApplication::applicationFilePath());
    QByteArray selfHash;

    if(!calculateFileChecksum(selfHash, selfEXE, QCryptographicHash::Sha256).wasSuccessful())
        return false;

    QString selfHashHex = String::fromByteArrayHex(selfHash);

    // Attempt to create unique mutex
    uniqueAppMutex = CreateMutex(NULL, FALSE, selfHashHex.toStdWString().c_str());
    if(GetLastError() == ERROR_ALREADY_EXISTS)
    {
        CloseHandle(uniqueAppMutex);
        return false;
    }

    // Instance is only one
    return true;
}

}
