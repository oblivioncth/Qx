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
    static const inline QString NULL_TARGET = QSL("<NULL>");
    static const inline QString TYPE_MACRO = QSL("<target>");
    static const inline QHash<IoOpTargetType, QString> TARGET_TYPE_STRINGS  = {
        {IO_FILE, QSL("file")},
        {IO_DIR, QSL("directory")}
    };
    static const inline QString SUCCESS_TEMPLATE = QSL(R"(Successfully %1 %2 "%3")");
    static const inline QString ERROR_TEMPLATE = QSL(R"(Error while %1 %2 "%3")");
    static const inline QHash<IoOpType, QString> SUCCESS_VERBS = {
        {IO_OP_READ, QSL("read")},
        {IO_OP_WRITE, QSL("wrote")},
        {IO_OP_ENUMERATE, QSL("enumerated")},
        {IO_OP_INSPECT, QSL("inspected")},
        {IO_OP_MANIPULATE, QSL("manipulated")}
    };
    static const inline QHash<IoOpType, QString> ERROR_VERBS = {
        {IO_OP_READ, QSL("reading")},
        {IO_OP_WRITE, QSL("writing")},
        {IO_OP_ENUMERATE, QSL("enumerating")},
        {IO_OP_INSPECT, QSL("inspecting")},
        {IO_OP_MANIPULATE, QSL("manipulating")}
    };
    static const inline QHash<IoOpResultType, QString> ERROR_INFO = {
        {IO_ERR_UNKNOWN, QSL("An unknown error has occurred.")},
        {IO_ERR_ACCESS_DENIED, QSL("Access denied.")},
        {IO_ERR_WRONG_TYPE, QSL("Target is not a ") + TYPE_MACRO + QSL(".")},
        {IO_ERR_OUT_OF_RES, QSL("Out of resources.")},
        {IO_ERR_READ, QSL("General read error.")},
        {IO_ERR_WRITE, QSL("General write error.")},
        {IO_ERR_FATAL, QSL("A fatal error has occurred.")},
        {IO_ERR_OPEN, QSL("Could not open ") + TYPE_MACRO + QSL(".")},
        {IO_ERR_ABORT, QSL("The operation was aborted.")},
        {IO_ERR_TIMEOUT, QSL("Request timed out.")},
        {IO_ERR_REMOVE, QSL("The ") + TYPE_MACRO + QSL(" could not be removed.")},
        {IO_ERR_RENAME, QSL("The ") + TYPE_MACRO + QSL(" could not be renamed.")},
        {IO_ERR_REPOSITION, QSL("The ") + TYPE_MACRO + QSL(" could not be moved.")},
        {IO_ERR_RESIZE, QSL("The ") + TYPE_MACRO + QSL(" could not be resized.")},
        {IO_ERR_COPY, QSL("The ") + TYPE_MACRO + QSL(" could not be copied.")},
        {IO_ERR_DNE, QSL("The ") + TYPE_MACRO + QSL(" does not exist.")},
        {IO_ERR_PATH_DNE, QSL("The containing path of the ") + TYPE_MACRO + QSL(" does not exist.")},
        {IO_ERR_NULL, QSL("The target is null")},
        {IO_ERR_EXISTS, QSL("The ") + TYPE_MACRO + QSL(" already exists.")},
        {IO_ERR_CANT_CREATE, QSL("The ") + TYPE_MACRO + QSL(" could not be created.")},
        {IO_ERR_CANT_CREATE_PATH, QSL("The path to the ") + TYPE_MACRO + QSL(" could not be created.")},
        {IO_ERR_FILE_SIZE_MISMATCH, QSL("File size mismatch.")},
        {IO_ERR_CURSOR_OOB, QSL("File data cursor has gone out of bounds.")},
        {IO_ERR_FILE_NOT_OPEN, QSL("The file is not open.")}
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
