// Unit Includes
#include "qx/windows-gui/qx-taskbarbutton.h"

// Intra-component Includes
#include "qx/windows-gui/qx-winguievent.h"
#include "qx-winguieventfilter_p.h"

// Extra-component Includes
#include "qx/windows/qx-common-windows.h"

namespace Qx
{

//===============================================================================================================
// TaskbarButton
//===============================================================================================================

/*!
 *  @class TaskbarButton
 *  @ingroup qx-windows-gui
 *
 *  @brief The TaskbarButton class represents the Windows taskbar button for a top-level window.
 *
 *  A TaskbarButton instance enables one to manipulate the overlay icon, overlay accessibility description,
 *  and progress indicator of the taskbar button that its connected window is associated with.
 *
 *  An overlay icon indicates change in the state of an application, whereas a progress indicator shows how
 *  time-consuming tasks are progressing.
 *
 *  @image{inline} html qx-taskbarbutton-0.png
 *
 *  The following example code illustrates how to use the TaskbarButton classes to adjust the look of the
 *  taskbar button:
 *
 *  @snippet qx-tasbarbutton.cpp 0
 *
 *  @note QWidget::windowHandle() returns a valid instance of a QWindow only after the widget has been shown.
 *  It is therefore recommended to delay the initialization of the TaskbarButton instances until
 *  QWidget::showEvent().
 *
 *  @par Progress:
 *
 *  @par
 *  A progress indicator is used to give the user an indication of the progress of an operation and to reassure
 *  them that the application is still running.
 *
 *  @par
 *  The progress indicator uses the concept of @e steps. It is set up by specifying the minimum and maximum
 *  possible step values, and it will display the percentage of steps that have been completed when you later
 *  give it the current step value. The percentage is calculated by dividing the progress
 *  (progressValue() - progressMinimum()) divided by progressMaximum() - progressMinimum().
 *
 *  @par
 *  The minimum and maximum number of steps can be specified by calling setProgressMinimum() and
 *  setProgressMaximum(). The current number of steps is set with setValue(). The progress indicator can be
 *  rewound to the beginning with resetProgress().
 *
 *  @par
 *  If minimumProgress and maximumProgress both are set to @c 0, the indicator state is automatically changed
 *  to ProgressState::Busy if it was previously ProgressState::Normal. This is useful when it is not possible
 *  to determine the number of steps. The progress state will be returned to normal when non-zero progress
 *  range is set and progress changes.
 *
 *  @par
 *  @table
 *  @row @li @image{inline} html qx-taskbarbutton-1.png Screenshot of a progress indicator
 *       @li A progress indicator at 50%.
 *  @row @li @image{inline} html qx-taskbarbutton-2.png Screenshot of a paused progress indicator
 *       @li A paused progress indicator at 50%.
 *  @row @li @image{inline} html qx-taskbarbutton-3.png Screenshot of a stopped progress indicator
 *      @li A stopped progress indicator at 50%.
 *  @row @li @image{inline} html qx-taskbarbutton-4.png Screenshot of an indeterminate progress indicator
 *       @li An indeterminate progress indicator.
 *  @endtable
 *
 *  @par
 *  @note The final appearance of the progress indicator varies depending on the active Windows theme.
 */

//-Class Enums-----------------------------------------------------------------------------------------------
//Public:
/*!
 *  @enum TaskbarButton::ProgressState
 *
 *  This enum represents the display state of a task bar button's progress indicator.
 */

/*!
 *  @var TaskbarButton::ProgressState TaskbarButton::Normal
 *  The progress indicator is in a normal state.
 */

/*!
 *  @var TaskbarButton::ProgressState TaskbarButton::Hidden
 *  The progress indicator is hidden.
 */

/*!
 *  @var TaskbarButton::ProgressState TaskbarButton::Stopped
 *  The progress indicator is stylized to indicate progress has been stopped.
 */

/*!
 *  @var TaskbarButton::ProgressState TaskbarButton::Paused
 *  The progress indicator is stylized to indicate progress has been paused.
 */

/*!
 *  @var TaskbarButton::ProgressState TaskbarButton::Busy
 *  The progress indicator is stylized to indicate progress is indeterminate.
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
    Constructs a TaskbarButton with the specified @a parent.

    If @a parent is an instance of QWindow, it is automatically assigned as the taskbar button's @l window.
 */
TaskbarButton::TaskbarButton(QObject *parent) :
    QObject(parent),
    mProgressValue(0),
    mProgressMinimum(0),
    mProgressMaximum(100),
    mProgressState(ProgressState::Hidden)
{
    // Ensure WinGuiEvent filter is installed
    WinGuiEventFilter::installGlobally();

    // Set the TaskbarButtons default associated window to that of the parent if available
    setWindow(qobject_cast<QWindow*>(parent));

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
/*!
 *  Destroys the TaskbarButton.
 */
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
/*!
 *  @internal
 *  Intercepts TaskbarButtonCreated messages.
 */
bool TaskbarButton::eventFilter(QObject* object, QEvent* event)
{
    if (object == mWindow && event->type() == WinGuiEvent::TaskbarButtonCreated)
    {
        updateProgressValue();
        updateOverlay();
    }
    return false;
}

/*!
 *  @property TaskbarButton::overlayIcon
 *  @brief the overlay icon of the taskbar button
 */
QIcon TaskbarButton::overlayIcon() const { return mOverlayIcon; }

/*!
 *  @property TaskbarButton::overlayAccessibleDescription
 *  @brief the description of the overlay for accessibility purposes
 *
 *  @sa overlayIcon
 */
QString TaskbarButton::overlayAccessibleDescription() const { return mOverlayAccessibleDescription; }

/*!
 *  @property TaskbarButton::window
 *  @brief the window whose taskbar button is manipulated
 */
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

/*!
 *   @property TaskbarButton::progressValue
 *   @brief the current value of the progress indicator
 *
 *  The default value is @c 0.
 */
int TaskbarButton::progressValue() const { return mProgressValue; }

/*!
 *   @property TaskbarButton::progressMinimum
 *   @brief the minimum value of the progress indicator
 *
 *  The default value is @c 0.
 */
int TaskbarButton::progressMinimum() const { return mProgressMinimum; }

/*!
 *   @property TaskbarButton::progressMaximum
 *   @brief the maximum value of the progress indicator
 *
 *  The default value is @c 100.
 */
int TaskbarButton::progressMaximum() const { return mProgressMaximum; }

/*!
 *   @property TaskbarButton::progressState
 *   @brief the display state of the progress indicator
 *
 *  The default value is @c ProgressState::Hidden.
 */
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

//-Signals------------------------------------------------------------------------------------------------
/*!
    @fn void QWinTaskbarProgress::progressStateChanged(Qx::TaskbarButton::ProgressState progressState)
    @internal (for QWinTaskbarButton and QML compatibility)
 */
}
