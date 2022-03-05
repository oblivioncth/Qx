#include "qx-ioopreport.h"

namespace Qx
{

//===============================================================================================================
// IoOpReport
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
IoOpReport::IoOpReport() :
    mNull(true),
    mOperation(IO_OP_ENUMERATE),
    mResult(IO_SUCCESS),
    mTargetType(IO_FILE),
    mTarget(QString())
{}

IoOpReport::IoOpReport(IoOpType op, IoOpResultType res, const QFile& tar) :
    mNull(false),
    mOperation(op),
    mResult(res),
    mTargetType(IO_FILE),
    mTarget(tar.fileName())
{
    parseOutcome();
}

IoOpReport::IoOpReport(IoOpType op, IoOpResultType res, const QDir& tar) :
     mNull(false),
     mOperation(op),
     mResult(res),
     mTargetType(IO_DIR),
     mTarget(tar.absolutePath())
{
    parseOutcome();
}


//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
IoOpType IoOpReport::operation() const { return mOperation; }
IoOpResultType IoOpReport::result() const { return mResult; }
IoOpTargetType IoOpReport::resultTargetType() const { return mTargetType; }
QString IoOpReport::target() const { return mTarget; }
QString IoOpReport::outcome() const { return mOutcome; }
QString IoOpReport::outcomeInfo() const { return mOutcomeInfo; }
bool IoOpReport::wasSuccessful() const { return mResult == IO_SUCCESS; }
bool IoOpReport::isNull() const { return mNull; }

//Private:
void IoOpReport::parseOutcome()
{
    if(mResult == IO_SUCCESS)
        mOutcome = SUCCESS_TEMPLATE.arg(SUCCESS_VERBS.value(mOperation), TARGET_TYPES.value(mTargetType), QDir::toNativeSeparators(mTarget));
    else
    {
        mOutcome = ERROR_TEMPLATE.arg(ERROR_VERBS.value(mOperation), TARGET_TYPES.value(mTargetType), QDir::fromNativeSeparators(mTarget));
        mOutcomeInfo = ERROR_INFO.value(mResult);
    }

}

}
