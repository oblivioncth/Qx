#include "qx-windows.h"

#include "qx-io.h"

#include <QFileInfo>
#include <QCoreApplication>

#include <tlhelp32.h>
#include "comdef.h"
#include "winnls.h"
#include "shobjidl.h"
#include "objbase.h"
#include "objidl.h"
#include "shlguid.h"
#include "atlbase.h"

namespace Qx
{

namespace  // Anonymous namespace for effectively private (to this cpp) functions
{
    // Alternate function to internal RtlNtStatusToDosError (which requires address linkage),
    // Thanks to https://gist.github.com/ian-abbott/732c5b88182a1941a603
    DWORD ConvertNtStatusToWin32Error(NTSTATUS ntstatus)
    {
            DWORD oldError;
            DWORD result;
            DWORD br;
            OVERLAPPED o;

            o.Internal = ntstatus;
            o.InternalHigh = 0;
            o.Offset = 0;
            o.OffsetHigh = 0;
            o.hEvent = 0;
            oldError = GetLastError();
            GetOverlappedResult(NULL, &o, &br, FALSE);
            result = GetLastError();
            SetLastError(oldError);
            return result;
    }
}

//-Classes------------------------------------------------------------------------------------------------------------

//===============================================================================================================
// FileDetails
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------------
//Public:
FileDetails::FileDetails() {}

//-Instance Functions------------------------------------------------------------------------------------------------
//Private:
bool FileDetails::isNull() { return mFileVersion.isNull() && mProductVersion.isNull() && mStringTables.isEmpty(); }
int FileDetails::stringTableCount() { return mStringTables.count(); }
QList<QPair<QString, QString>> FileDetails::availableLangCodePages() { return mLangCodePageMap.keys(); }
bool FileDetails::hasLangCodePage(QString lanuage, QString codePage) { return mLangCodePageMap.contains(qMakePair(lanuage.toUpper(), codePage.toUpper())); }
Mmrb FileDetails::metaStructVersion() { return mMetaStructVersion; }

Mmrb FileDetails::fileVersion() { return mFileVersion; }
Mmrb FileDetails::productVersion() { return mProductVersion; }
FileDetails::FileFlags FileDetails::fileFlags() { return mFileFlags; }
FileDetails::TargetSystems FileDetails::targetSystems() { return mTargetSystems; }
FileDetails::FileType FileDetails::fileType() { return mFileType; }
FileDetails::FileSubType FileDetails::fileSubType() { return mFileSubType; }
int FileDetails::virtualDeviceID() { return mVirtualDeviceID; }

const FileDetails::StringTable FileDetails::stringTable(int index)
{
    if(index >= 0 && index < mStringTables.count())
        return mStringTables.at(index);
    else
        return FileDetails::StringTable();
}

const FileDetails::StringTable FileDetails::stringTable(QString language, QString codePage)
{
    if(hasLangCodePage(language.toUpper(), codePage.toUpper()))
        return mStringTables.at(mLangCodePageMap.value(qMakePair(language.toUpper(), codePage.toUpper())));
    else
        return StringTable();
}

//Public:
void FileDetails::addStringTable(StringTable stringTable)
{
    mStringTables.append(stringTable);
    mLangCodePageMap[qMakePair(stringTable.metaLanguageID, stringTable.metaCodePageID)] = mStringTables.count() - 1;
}

//-Functions-------------------------------------------------------------------------------------------------------------
FileDetails readFileDetails(QString filePath)
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
                            workingFileDetails.mMetaStructVersion = Mmrb(fixedFileInfo->dwStrucVersion >> 16,
                                                                         fixedFileInfo->dwStrucVersion & 0xFFFF,
                                                                         0, 0);

                            // Get file version
                            workingFileDetails.mFileVersion = Mmrb(fixedFileInfo->dwFileVersionMS >> 16,
                                                                   fixedFileInfo->dwFileVersionMS & 0xFFFF,
                                                                   fixedFileInfo->dwFileVersionLS >> 16,
                                                                   fixedFileInfo->dwFileVersionLS & 0xFFFF);

                            // Get product version
                            workingFileDetails.mProductVersion = Mmrb(fixedFileInfo->dwProductVersionMS >> 16,
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

DWORD processIDByName(QString processName)
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

QString processNameByID(DWORD processID)
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

bool processIsRunning(QString processName) { return processIDByName(processName); }
bool processIsRunning(DWORD processID) { return processNameByID(processID).isNull(); }

bool enforceSingleInstance()
{
    // Presistant handle instance
    static HANDLE uniqueAppMutex = NULL;

    // Get self hash
    QFile selfEXE(QCoreApplication::applicationFilePath());
    QString selfHash;

    if(!calculateFileChecksum(selfHash, selfEXE, QCryptographicHash::Sha256).wasSuccessful())
        return false;

    // Attempt to create unique mutex
    uniqueAppMutex = CreateMutex(NULL, FALSE, (const wchar_t*)selfHash.utf16());
    if(GetLastError() == ERROR_ALREADY_EXISTS)
    {
        CloseHandle(uniqueAppMutex);
        return false;
    }

    // Instance is only one
    return true;
}

Qx::GenericError translateHresult(HRESULT res)
{
    BitArray resBits = BitArray::fromInteger(res);

    // Check if result is actually an ntstatus code
    if(resBits.testBit(28))
        return translateNtstatus(res);

    // Check for success
    if(!resBits.testBit(31))
        return Qx::GenericError();

    // Create com error instance from result
    _com_error comError(res);

    // Return translated error
    return Qx::GenericError(GenericError::Error, QString::fromWCharArray(comError.ErrorMessage()));
}

Qx::GenericError translateNtstatus(NTSTATUS stat)
{
    BitArray statBits = BitArray::fromInteger(stat);

    // Get severity
    BitArray severityBits = statBits.extract(30, 2);
    quint8 severity = severityBits.toInteger<quint8>();

    // Check for success
    if(severity == 0x00)
        return Qx::GenericError();

    // Get handle to ntdll.dll.
    HMODULE hNtDll = LoadLibrary(L"NTDLL.DLL");

    // Return unknown error if library fails to load
    if (hNtDll == NULL)
        return GenericError::UNKNOWN_ERROR;

    // Use format message to create error string
    TCHAR* formatedBuffer = nullptr;
    DWORD formatResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
                                       hNtDll, ConvertNtStatusToWin32Error(stat), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                       (LPTSTR)&formatedBuffer, 0, NULL);

    // Free loaded dll
    FreeLibrary(hNtDll);

    // Return unknown error if message fails to format
    if(!formatResult)
        return GenericError::UNKNOWN_ERROR;

    // Return translated error
    return Qx::GenericError(severity == 0x03 ? GenericError::Error : GenericError::Warning, QString::fromWCharArray(formatedBuffer));
}

Qx::GenericError createShortcut(QString shortcutPath, ShortcutProperties sp)
{
    // Check for basic argument validity
    if(sp.target.isEmpty() || shortcutPath.isEmpty() || sp.iconIndex < 0)
        return translateHresult(E_INVALIDARG);

    // Working vars
    HRESULT hRes;
    CComPtr<IShellLink> ipShellLink;

    // Get full path of target
    QFileInfo targetInfo(sp.target);
    QString fullTargetPath = targetInfo.absoluteFilePath();

    // Get a pointer to the IShellLink interface
    hRes = CoCreateInstance(CLSID_ShellLink,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_IShellLink,
                            (void**)&ipShellLink);

    if (SUCCEEDED(hRes))
    {
        // Get a pointer to the IPersistFile interface
        CComQIPtr<IPersistFile> ipPersistFile(ipShellLink);

        // Set shortcut properties
        hRes = ipShellLink->SetPath((const wchar_t*)fullTargetPath.utf16());
        if (FAILED(hRes))
            return translateHresult(hRes);

        if(!sp.targetArgs.isEmpty())
        {
            hRes = ipShellLink->SetArguments((const wchar_t*)sp.targetArgs.utf16());
            if (FAILED(hRes))
                return translateHresult(hRes);
        }

        if(!sp.startIn.isEmpty())
        {
            hRes = ipShellLink->SetWorkingDirectory((const wchar_t*)sp.startIn.utf16());
            if (FAILED(hRes))
                return translateHresult(hRes);
        }

        if(!sp.comment.isEmpty())
        {
            hRes = ipShellLink->SetDescription((const wchar_t*)sp.comment.utf16());
            if (FAILED(hRes))
                return translateHresult(hRes);
        }

        if(!sp.iconFilePath.isEmpty())
        {
            hRes = ipShellLink->SetIconLocation((const wchar_t*)sp.iconFilePath.utf16(), sp.iconIndex);
            if (FAILED(hRes))
                return translateHresult(hRes);
        }

        hRes = ipShellLink->SetShowCmd(sp.showMode);
        if (FAILED(hRes))
            return translateHresult(hRes);

        // Write the shortcut to disk
        hRes = ipPersistFile->Save((const wchar_t*)shortcutPath.utf16(), TRUE);
    }

    return translateHresult(hRes);
}

}
