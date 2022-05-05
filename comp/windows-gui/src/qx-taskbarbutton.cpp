// Unit Includes
#include "qx/windows-gui/qx-taskbarbutton.h"

// Windows Includes
#define NOMINMAX
#include "ShObjIdl_core.h"
#undef NOMINMAX

// Extra-component Includes
#include "qx/windows/qx-common-windows.h"

namespace Qx
{

//===============================================================================================================
// TaskbarButton
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
TaskbarButton::TaskbarButton(QObject *parent) :
    QObject(parent),
    mProgressValue(0),
    mProgressMinimum(0),
    mProgressMaximum(100),
    mProgressState(ProgressState::Hidden)
{
    // Acquire Taskbar Interface
    HRESULT hresult = CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskbarList4, reinterpret_cast<void **>(&mTaskbarInterface));
    if (FAILED(hresult))
    {
        mTaskbarInterface = nullptr;

        const GenericError err = Qx::translateHresult(hresult);
        QString errStr;
        QTextStream es(&errStr);
        es << err;
        qWarning("TaskbarButton: ITaskbarList4 interface was not created.\n%s.", qPrintable(errStr));
    }
    else if (FAILED(mTaskbarInterface->HrInit()))
    {
        mTaskbarInterface->Release();
        mTaskbarInterface = nullptr;

        const GenericError err = Qx::translateHresult(hresult);
        QString errStr;
        QTextStream es(&errStr);
        es << err;
        qWarning("TaskbarButton: ITaskbarList4 interface was not initialized.\n%s", unsigned(hresult), qPrintable(errStr));
    }
}

//-Destructor---------------------------------------------------------------------------------------------------
//Public:
TaskbarButton::~TaskbarButton()
{
    if (mTaskbarInterface)
        mTaskbarInterface->Release();
}

//-Class Functions----------------------------------------------------------------------------------------------
//Private:
int TaskbarButton::getNativeProgressState(ProgressState progressState)
{
    switch(progressState)
    {
        case TaskbarButton::Normal:
            return TBPF_NORMAL;
        case TaskbarButton::Hidden:
            return TBPF_NOPROGRESS;
        case TaskbarButton::Stopped:
            return TBPF_ERROR;
        case TaskbarButton::Paused:
            return TBPF_PAUSED;
        case TaskbarButton::Busy:
            return TBPF_INDETERMINATE;
        default:
            return TBPF_NOPROGRESS;
    }
}

//-Instance Functions----------------------------------------------------------------------------------------------
//Private:
void TaskbarButton::updateProgressValue()
{
    if(!window)
        return;

    // Check for actual range
    const int progressRange = mProgressMaximum - mProgressMinimum;
    if(progressRange > 0)
    {
        // Cancel indeterminate state if applicable
        if(mProgressState == ProgressState::Busy)
            mProgressState = ProgressState::Normal;

        // Scale value to 0-100 due to WinAPI limitations
        const int scaledValue = std::round(double(100) * (double(mProgressValue - mProgressMinimum)) / double(progressRange));
        mTaskbarInterface->SetProgressValue(handle(), ULONGLONG(scaledValue), 100);
    }
    else if(mProgressState == ProgressState::Normal) // Auto busy state
        mProgressState = ProgressState::Busy;

    // Reinforce progress state since SetProgressValue can change it
    updateProgressState();
}

void TaskbarButton::updateProgressState()
{
    mTaskbarInterface->SetProgressState(handle(), getNativeProgressState(mProgressState));
}

//Public:
int TaskbarButton::progressValue() const { return mProgressValue; }
int TaskbarButton::progressMinimum() const { return mProgressMinimum; }
int TaskbarButton::progressMaximum() const { return mProgressMaximum; }
TaskbarButton::ProgressState TaskbarButton::progressState() const { return mProgressState; }

//-Slots------------------------------------------------------------------------------------------------------------
//Public:
void TaskbarButton::setProgressValue(int progressValue)
{
    if(progressValue == mProgressValue || progressValue < mProgressMinimum || progressValue > mProgressMaximum)
        return;

    mProgressValue = progressValue;
    updateProgressValue();
    emit progressValueChanged(mProgressValue);
}

void TaskbarButton::setProgressMinimum(int progressMinimum) { setProgressRange(progressMinimum, std::max(progressMinimum, mProgressMaximum)); }

void TaskbarButton::setProgressMaximum(int progressMaximum) { setProgressRange(std::min(mProgressMinimum, progressMaximum), progressMaximum); }

void TaskbarButton::setProgressRange(int progressMinimum, int progressMaximum)
{
    const bool minChanged = progressMinimum != mProgressMinimum;
    const bool maxChanged = progressMaximum != mProgressMaximum;

    if(!minChanged && !maxChanged)
        return;

    mProgressMinimum = progressMinimum;
    mProgressMaximum = std::max(progressMinimum, progressMaximum);

    if (mProgressValue < mProgressMinimum || mProgressValue > mProgressMaximum)
        resetProgress();

    updateProgressValue();

    if (minChanged)
        emit progressMinimumChanged(mProgressMinimum);
    if (maxChanged)
        emit progressMaximumChanged(mProgressMaximum);
}

void TaskbarButton::setProgressState(Qx::TaskbarButton::ProgressState progressState)
{
    if(progressState == mProgressState)
        return;

    mProgressState = progressState;
    updateProgressState();
    emit progressStateChanged(mProgressState);
}

void TaskbarButton::resetProgress() { setProgressValue(mProgressMinimum); }

}
