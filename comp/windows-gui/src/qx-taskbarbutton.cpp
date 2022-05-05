// Unit Includes
#include "qx/windows-gui/qx-taskbarbutton.h"

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

//-Instance Functions----------------------------------------------------------------------------------------------
//Private:
HWND TaskbarButton::getNativeWindowHandle()
{
    return mWindow ? reinterpret_cast<HWND>(mWindow->winId()) : 0;
}

int TaskbarButton::getNativeIconSize() { return GetSystemMetrics(SM_CXSMICON); }

void TaskbarButton::updateOverlay()
{
    if (!mWindow)
        return;

    // Initial native references
    LPCWSTR description = nullptr;
    HICON iconHandle = nullptr;

    // Get description if possible
    if (!mOverlayAccessibleDescription.isEmpty())
        description = mOverlayAccessibleDescription.toStdWString().c_str();

    // Get icon if possible
    if (!mOverlayIcon.isNull())
    {
        // Attempt to convert to native icon
        iconHandle = mOverlayIcon.pixmap(getNativeIconSize()).toImage().toHICON();

        // Fallback to application default icon
        if(!iconHandle)
            iconHandle = static_cast<HICON>(LoadImage(nullptr, IDI_APPLICATION, IMAGE_ICON, SM_CXSMICON, SM_CYSMICON, LR_SHARED));
    }

    // Set icon and description
    mTaskbarInterface->SetOverlayIcon(getNativeWindowHandle(), iconHandle, description);

    // Free converted icon if present (SetOverlayIcon creates a copy)
    if (iconHandle)
        DestroyIcon(iconHandle);
}

void TaskbarButton::updateProgressValue()
{
    if(!mWindow)
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
        mTaskbarInterface->SetProgressValue(getNativeWindowHandle(), ULONGLONG(scaledValue), 100);
    }
    else if(mProgressState == ProgressState::Normal) // Auto busy state
        mProgressState = ProgressState::Busy;

    // Reinforce progress state since SetProgressValue can change it
    updateProgressState();
}

void TaskbarButton::updateProgressState()
{
    mTaskbarInterface->SetProgressState(getNativeWindowHandle(),
                                        static_cast<TBPFLAG>(mProgressState));
}

//Public:
bool TaskbarButton::eventFilter(QObject* object, QEvent* event)
{
    if (object == mWindow && event->type() == QWinEvent::TaskbarButtonCreated)
    {
        updateProgressValue();
        updateOverlay();
    }
    return false;
}

QIcon TaskbarButton::overlayIcon() const { return mOverlayIcon; }
QString TaskbarButton::overlayAccessibleDescription() const { return mOverlayAccessibleDescription; }

QWindow* TaskbarButton::window() const { return mWindow; }

void TaskbarButton::setWindow(QWindow* window)
{
    // Remove existing event filter if present
    if (mWindow)
        mWindow->removeEventFilter(this);

    // Update parent window
    mWindow = window;

    // Update progress display and overlay if window isn't null
    if (mWindow)
    {
        mWindow->installEventFilter(this);
        if (mWindow->isVisible())
        {
            updateProgressValue();
            updateOverlay();
        }
    }
}

int TaskbarButton::progressValue() const { return mProgressValue; }
int TaskbarButton::progressMinimum() const { return mProgressMinimum; }
int TaskbarButton::progressMaximum() const { return mProgressMaximum; }
TaskbarButton::ProgressState TaskbarButton::progressState() const { return mProgressState; }

//-Slots------------------------------------------------------------------------------------------------------------
//Public:
void TaskbarButton::setOverlayIcon(const QIcon& icon)
{
    mOverlayIcon = icon;
    updateOverlay();
}

void TaskbarButton::setOverlayAccessibleDescription(const QString& description)
{
    mOverlayAccessibleDescription = description;
    updateOverlay();
}

void TaskbarButton::clearOverlayIcon()
{
    setOverlayAccessibleDescription(QString());
    setOverlayIcon(QIcon());
}

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
