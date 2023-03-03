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
#include "qx/core/qx-genericerror.h"

namespace Qx
{
	
//-Types------------------------------------------------------------------------------------------------------
enum IoOpType { IO_OP_READ, IO_OP_WRITE, IO_OP_ENUMERATE, IO_OP_INSPECT, IO_OP_MANIPULATE };
enum IoOpResultType { IO_SUCCESS, IO_ERR_UNKNOWN, IO_ERR_ACCESS_DENIED, IO_ERR_WRONG_TYPE, IO_ERR_OUT_OF_RES,
                      IO_ERR_READ, IO_ERR_WRITE, IO_ERR_FATAL, IO_ERR_OPEN, IO_ERR_ABORT,
                      IO_ERR_TIMEOUT, IO_ERR_REMOVE, IO_ERR_RENAME, IO_ERR_REPOSITION,
                      IO_ERR_RESIZE, IO_ERR_COPY, IO_ERR_DNE, IO_ERR_NULL,
                      IO_ERR_EXISTS, IO_ERR_CANT_CREATE, IO_ERR_FILE_SIZE_MISMATCH, IO_ERR_CURSOR_OOB,
                      IO_ERR_FILE_NOT_OPEN};
enum IoOpTargetType { IO_FILE, IO_DIR };


//-Classes--------------------------------------------------------------------------------------------
class QX_IO_EXPORT IoOpReport
{
//-Class Members----------------------------------------------------------------------------------------------------
private:
    static const inline QString NULL_TARGET = "<NULL>";
    static const inline QString TYPE_MACRO = "<target>";
    static const inline QHash<IoOpTargetType, QString> TARGET_TYPE_STRINGS  = {
        {IO_FILE, "file"},
        {IO_DIR, "directory"}
    };
    static const inline QString SUCCESS_TEMPLATE = R"(Successfully %1 %2 "%3")";
    static const inline QString ERROR_TEMPLATE = R"(Error while %1 %2 "%3")";
    static const inline QHash<IoOpType, QString> SUCCESS_VERBS = {
        {IO_OP_READ, "read"},
        {IO_OP_WRITE, "wrote"},
        {IO_OP_ENUMERATE, "enumerated"},
        {IO_OP_INSPECT, "inspected"},
        {IO_OP_MANIPULATE, "manipulated"}
    };
    static const inline QHash<IoOpType, QString> ERROR_VERBS = {
        {IO_OP_READ, "reading"},
        {IO_OP_WRITE, "writing"},
        {IO_OP_ENUMERATE, "enumerating"},
        {IO_OP_INSPECT, "inspecting"},
        {IO_OP_MANIPULATE, "manipulating"}
    };
    static const inline QHash<IoOpResultType, QString> ERROR_INFO = {
        {IO_ERR_UNKNOWN, "An unknown error has occurred."},
        {IO_ERR_ACCESS_DENIED, "Access denied."},
        {IO_ERR_WRONG_TYPE, "Target is not a " + TYPE_MACRO + "."},
        {IO_ERR_OUT_OF_RES, "Out of resources."},
        {IO_ERR_READ, "General read error."},
        {IO_ERR_WRITE, "General write error."},
        {IO_ERR_FATAL, "A fatal error has occurred."},
        {IO_ERR_OPEN, "Could not open " + TYPE_MACRO + "."},
        {IO_ERR_ABORT, "The operation was aborted."},
        {IO_ERR_TIMEOUT, "Request timed out."},
        {IO_ERR_REMOVE, "The " + TYPE_MACRO + " could not be removed."},
        {IO_ERR_RENAME, "The " + TYPE_MACRO + " could not be renamed."},
        {IO_ERR_REPOSITION, "The " + TYPE_MACRO + " could not be moved."},
        {IO_ERR_RESIZE, "The " + TYPE_MACRO + " could not be resized."},
        {IO_ERR_COPY, "The " + TYPE_MACRO + " could not be copied."},
        {IO_ERR_DNE, "The " + TYPE_MACRO + " does not exist."},
        {IO_ERR_NULL, "The target is null"},
        {IO_ERR_EXISTS, "The " + TYPE_MACRO + " already exists."},
        {IO_ERR_CANT_CREATE, "The " + TYPE_MACRO + " could not be created."},
        {IO_ERR_FILE_SIZE_MISMATCH, "File size mismatch."},
        {IO_ERR_CURSOR_OOB, "File data cursor has gone out of bounds."},
        {IO_ERR_FILE_NOT_OPEN, "The file is not open."}
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

public:
    IoOpType operation() const;
    IoOpResultType result() const;
    IoOpTargetType resultTargetType() const;
    QString target() const;
    QString outcome() const;
    QString outcomeInfo() const;
    bool isFailure() const;
    bool isNull() const;
    GenericError toGenericError() const;
};

}

#endif // QX_IOOPREPORT_H
