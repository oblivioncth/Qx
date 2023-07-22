#ifndef QX_IOOPREPORT_H
#define QX_IOOPREPORT_H

// Shared Lib Support
#include "qx/io/qx_io_export.h"

// Qt Includes
#include <QStringList>
#include <QHash>
#include <QFile>
#include <QDir>

// Inter-project Includes
#include "qx/core/qx-abstracterror.h"

namespace Qx
{
	
//-Types------------------------------------------------------------------------------------------------------
enum IoOpType { IO_OP_READ, IO_OP_WRITE, IO_OP_ENUMERATE, IO_OP_INSPECT, IO_OP_MANIPULATE };
enum IoOpResultType {
    IO_SUCCESS = 0,
    IO_ERR_UNKNOWN = 1,
    IO_ERR_ACCESS_DENIED = 2,
    IO_ERR_WRONG_TYPE = 3,
    IO_ERR_OUT_OF_RES = 4,
    IO_ERR_READ = 5,
    IO_ERR_WRITE = 6,
    IO_ERR_FATAL = 7,
    IO_ERR_OPEN = 8,
    IO_ERR_ABORT = 9,
    IO_ERR_TIMEOUT = 10,
    IO_ERR_REMOVE = 11,
    IO_ERR_RENAME = 12,
    IO_ERR_REPOSITION = 13,
    IO_ERR_RESIZE = 14,
    IO_ERR_COPY = 15,
    IO_ERR_DNE = 16,
    IO_ERR_PATH_DNE = 17,
    IO_ERR_NULL = 18,
    IO_ERR_EXISTS = 19,
    IO_ERR_CANT_CREATE = 20,
    IO_ERR_CANT_CREATE_PATH = 21,
    IO_ERR_FILE_SIZE_MISMATCH = 22,
    IO_ERR_CURSOR_OOB = 23,
    IO_ERR_FILE_NOT_OPEN = 24
};
enum IoOpTargetType { IO_FILE, IO_DIR };


//-Classes--------------------------------------------------------------------------------------------
class QX_IO_EXPORT IoOpReport final : public AbstractError<"Qx::IoOpReport", 1>
{
//-Class Members----------------------------------------------------------------------------------------------------
private:
    static const inline QString NULL_TARGET = u"<NULL>"_s;
    static const inline QString TYPE_MACRO = u"<target>"_s;
    static const inline QHash<IoOpTargetType, QString> TARGET_TYPE_STRINGS  = {
        {IO_FILE, u"file"_s},
        {IO_DIR, u"directory"_s}
    };
    static const inline QString SUCCESS_TEMPLATE = uR"(Successfully %1 %2 "%3")"_s;
    static const inline QString ERROR_TEMPLATE = uR"(Error while %1 %2 "%3")"_s;
    static const inline QHash<IoOpType, QString> SUCCESS_VERBS = {
        {IO_OP_READ, u"read"_s},
        {IO_OP_WRITE, u"wrote"_s},
        {IO_OP_ENUMERATE, u"enumerated"_s},
        {IO_OP_INSPECT, u"inspected"_s},
        {IO_OP_MANIPULATE, u"manipulated"_s}
    };
    static const inline QHash<IoOpType, QString> ERROR_VERBS = {
        {IO_OP_READ, u"reading"_s},
        {IO_OP_WRITE, u"writing"_s},
        {IO_OP_ENUMERATE, u"enumerating"_s},
        {IO_OP_INSPECT, u"inspecting"_s},
        {IO_OP_MANIPULATE, u"manipulating"_s}
    };
    static const inline QHash<IoOpResultType, QString> ERROR_INFO = {
        {IO_ERR_UNKNOWN, u"An unknown error has occurred."_s},
        {IO_ERR_ACCESS_DENIED, u"Access denied."_s},
        {IO_ERR_WRONG_TYPE, u"Target is not a "_s + TYPE_MACRO + u"."_s},
        {IO_ERR_OUT_OF_RES, u"Out of resources."_s},
        {IO_ERR_READ, u"General read error."_s},
        {IO_ERR_WRITE, u"General write error."_s},
        {IO_ERR_FATAL, u"A fatal error has occurred."_s},
        {IO_ERR_OPEN, u"Could not open "_s + TYPE_MACRO + u"."_s},
        {IO_ERR_ABORT, u"The operation was aborted."_s},
        {IO_ERR_TIMEOUT, u"Request timed out."_s},
        {IO_ERR_REMOVE, u"The "_s + TYPE_MACRO + u" could not be removed."_s},
        {IO_ERR_RENAME, u"The "_s + TYPE_MACRO + u" could not be renamed."_s},
        {IO_ERR_REPOSITION, u"The "_s + TYPE_MACRO + u" could not be moved."_s},
        {IO_ERR_RESIZE, u"The "_s + TYPE_MACRO + u" could not be resized."_s},
        {IO_ERR_COPY, u"The "_s + TYPE_MACRO + u" could not be copied."_s},
        {IO_ERR_DNE, u"The "_s + TYPE_MACRO + u" does not exist."_s},
        {IO_ERR_PATH_DNE, u"The containing path of the "_s + TYPE_MACRO + u" does not exist."_s},
        {IO_ERR_NULL, u"The target is null"_s},
        {IO_ERR_EXISTS, u"The "_s + TYPE_MACRO + u" already exists."_s},
        {IO_ERR_CANT_CREATE, u"The "_s + TYPE_MACRO + u" could not be created."_s},
        {IO_ERR_CANT_CREATE_PATH, u"The path to the "_s + TYPE_MACRO + u" could not be created."_s},
        {IO_ERR_FILE_SIZE_MISMATCH, u"File size mismatch."_s},
        {IO_ERR_CURSOR_OOB, u"File data cursor has gone out of bounds."_s},
        {IO_ERR_FILE_NOT_OPEN, u"The file is not open."_s}
    };
    /* TODO: In the many const QHashs like this throughout the lib, figure out how to also
     * make the values const (for since they aren't to be modified), because as is they don't
     * compile that way.
     */

//-Instance Members-------------------------------------------------------------------------------------------------
private:
    bool mNull;
    IoOpType mOperation;
    IoOpResultType mResult;
    IoOpTargetType mTargetType;
    QString mTarget;
    QString mOutcome;
    QString mOutcomeInfo;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    IoOpReport();
    IoOpReport(IoOpType op, IoOpResultType res, const QFileDevice& tar);
    IoOpReport(IoOpType op, IoOpResultType res, const QFileDevice* tar);
    IoOpReport(IoOpType op, IoOpResultType res, const QDir& tar);
    IoOpReport(IoOpType op, IoOpResultType res, const QDir* tar);
    IoOpReport(IoOpType op, IoOpResultType res, const QFileInfo& tar);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void parseOutcome();

    quint32 deriveValue() const override;
    QString derivePrimary() const override;
    QString deriveSecondary() const override;

public:
    IoOpType operation() const;
    IoOpResultType result() const;
    IoOpTargetType resultTargetType() const;
    QString target() const;
    QString outcome() const;
    QString outcomeInfo() const;
    bool isFailure() const;
    bool isNull() const;
};

}

#endif // QX_IOOPREPORT_H
