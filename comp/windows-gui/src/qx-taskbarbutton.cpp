// Unit Includes
#include "qx/windows-gui/qx-taskbarbutton.h"

// Windows Includes
#include "qx_windows.h"

// Intra-component Includes
#include "qx/windows-gui/qx-winguievent.h"
#include "qx-winguieventfilter_p.h"

// Extra-component Includes
#include "qx/windows/qx-common-windows.h"

namespace Qx
{

namespace  // Anonymous namespace for effectively private (to this cpp) functions
{
    TBPFLAG getNativeProgressState(TaskbarButton::ProgressState ps)
    {
        switch(ps)
        {
            case TaskbarButton::ProgressState::Hidden:
                return TBPF_NOPROGRESS;
            case TaskbarButton::ProgressState::Busy:
                return TBPF_INDETERMINATE;
            case TaskbarButton::ProgressState::Normal:
                return TBPF_NORMAL;
            case TaskbarButton::ProgressState::Stopped:
                return TBPF_ERROR;
            case TaskbarButton::ProgressState::Paused:
                return TBPF_PAUSED;
            default:
                return TBPF_NOPROGRESS;
        }
    }
}

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
 *  The following example code illustrates how to use the TaskbarButton class to adjust the look of the
 *  taskbar button:
 *
 *  @snippet qx-taskbarbutton.cpp 0
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
 *  setProgressMaximum(). The current number of steps is set with setProgressValue(). The progress indicator can be
 *  rewound to the beginning with resetProgress().
 *
 *  @par
 *  If @ref progressMinimum and @ref progressMaximum both are set to @c 0, the indicator state is automatically
 *  changed to @ref Busy if it was previously @ref Normal. This is useful when it is
 *  not possible to determine the number of steps. The progress state will be returned to normal when non-zero
 *  progress range is set and progress changes.
 *
 *  @par
 *  <table>
 *  <caption>The different progress states of a taskbar button (at 50% progress)</caption>
 *  <tr><th>Progress State  <th>Appearance
 *  <tr><td>Hidden          <td>@image{inline} html qx-taskbarbutton-1.png
 *  <tr><td>Busy            <td>@image{inline} html qx-taskbarbutton-2.png
 *  <tr><td>Normal          <td>@image{inline} html qx-taskbarbutton-3.png
 *  <tr><td>Stopped         <td>@image{inline} html qx-taskbarbutton-4.png
 *  <tr><td>Paused          <td>@image{inline} html qx-taskbarbutton-5.png
 *  </table>
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

//-Class Properties-----------------------------------------------------------------------------------------------
//Private:
/*!
 *  @property TaskbarButton::overlayIcon
 *  @brief The overlay icon of the taskbar button
 */

 /*!
 *  @property TaskbarButton::overlayAccessibleDescription
 *  @brief The description of the overlay for accessibility purposes
 *
 *  @sa overlayIcon
 */

 /*!
 *  @property TaskbarButton::window
 *  @brief The window whose taskbar button is manipulated
 */

 /*!
 *   @property TaskbarButton::progressValue
 *   @brief The current value of the progress indicator
 *
 *  The default value is @c 0.
 */

 /*!
 *   @property TaskbarButton::progressMinimum
 *   @brief The minimum value of the progress indicator
 *
 *  The default value is @c 0.
 */

 /*!
 *   @property TaskbarButton::progressMaximum
 *   @brief The maximum value of the progress indicator
 *
 *  The default value is @c 100.
 */

 /*!
 *   @property TaskbarButton::progressState
 *   @brief The display state of the progress indicator
 *
 *  The default value is @c ProgressState::Hidden.
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs a TaskbarButton with the specified @a parent.
 *
 *  If @a parent is an instance of QWindow, it is automatically assigned as the taskbar button's @ref window.
 */
TaskbarButton::TaskbarButton(QObject *parent) :
    QObject(parent),
    mWindow(nullptr),
    mTaskbarInterface(nullptr),
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
    std::wstring description;
    HICON iconHandle = nullptr;

    // Get description if possible
    if (!mOverlayAccessibleDescription.isEmpty())
        description = mOverlayAccessibleDescription.toStdWString();

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
    mTaskbarInterface->SetOverlayIcon(getNativeWindowHandle(), iconHandle, description.c_str());

    // Free converted icon if present (SetOverlayIcon creates a copy)
    if (iconHandle)
        DestroyIcon(iconHandle);
}

void TaskbarButton::updateProgressIndicator()
{
    if(!mWindow)
        return;

    // Check for actual range
    const int progressRange = mProgressMaximum - mProgressMinimum;
    if(progressRange > 0)
    {
        // Scale value to 0-100 due to WinAPI limitations
        const int scaledValue = std::round(double(100) * (double(mProgressValue - mProgressMinimum)) / double(progressRange));
        mTaskbarInterface->SetProgressValue(getNativeWindowHandle(), ULONGLONG(scaledValue), 100);
    }
    else if(mProgressState == ProgressState::Normal) // Auto busy state
        mProgressState = ProgressState::Busy;

    // Reinforce progress state since SetProgressValue can change it
    mTaskbarInterface->SetProgressState(getNativeWindowHandle(),
                                        getNativeProgressState(mProgressState));
}

//Public:
/*! @cond */ // Implementation detail
bool TaskbarButton::eventFilter(QObject* object, QEvent* event)
{
    if (object == mWindow && event->type() == WinGuiEvent::TaskbarButtonCreated)
    {
        updateProgressIndicator();
        updateOverlay();
    }
    return false;
}
/*! @endcond */

/*!
 *  Returns the taskbar button's current icon overlay.
 */
QIcon TaskbarButton::overlayIcon() const { return mOverlayIcon; }

/*!
 *  Returns the taskbar button's current overlay description.
 */
QString TaskbarButton::overlayAccessibleDescription() const { return mOverlayAccessibleDescription; }


/*!
 *  Returns the window that the taskbar button is currently associated with.
 */
QWindow* TaskbarButton::window() const { return mWindow; }

/*!
 *  Sets the taskbar button's associated window.
 */
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
            updateProgressIndicator();
            updateOverlay();
        }
    }
}

/*!
 *  Returns the current value of the taskbar button's progress indicator.
 */
int TaskbarButton::progressValue() const { return mProgressValue; }

/*!
 *  Returns the minimum value of the taskbar button's progress indicator.
 */
int TaskbarButton::progressMinimum() const { return mProgressMinimum; }

/*!
 *  Returns the maximum value of the taskbar button's progress indicator.
 */
int TaskbarButton::progressMaximum() const { return mProgressMaximum; }

/*!
 *  Returns the display state of the taskbar button's progress indicator.
 */
TaskbarButton::ProgressState TaskbarButton::progressState() const { return mProgressState; }

//-Slots------------------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Sets the taskbar button's icon overlay to @a icon.
 */
void TaskbarButton::setOverlayIcon(const QIcon& icon)
{
    mOverlayIcon = icon;
    updateOverlay();
}

/*!
 *  Sets the taskbar button's icon overlay description to @a description.
 */
void TaskbarButton::setOverlayAccessibleDescription(const QString& description)
{
    mOverlayAccessibleDescription = description;
    updateOverlay();
}

/*!
 *  Clears the taskbar button's icon overlay and its description, if any.
 */
void TaskbarButton::clearOverlayIcon()
{
    setOverlayAccessibleDescription(QString());
    setOverlayIcon(QIcon());
}

/*!
 *  Sets the current value of the taskbar button's progress indicator.
 */
void TaskbarButton::setProgressValue(int progressValue)
{
    if(progressValue == mProgressValue || progressValue < mProgressMinimum || progressValue > mProgressMaximum)
        return;

    // Cancel indeterminate state if applicable
    if(mProgressState == ProgressState::Busy)
        mProgressState = ProgressState::Normal;

    mProgressValue = progressValue;
    updateProgressIndicator();
    emit progressValueChanged(mProgressValue);
}

/*!
 *  Sets the minimum value of the taskbar button's progress indicator.
 */
void TaskbarButton::setProgressMinimum(int progressMinimum) { setProgressRange(progressMinimum, std::max(progressMinimum, mProgressMaximum)); }

/*!
 *  Sets the maximum value of the taskbar button's progress indicator.
 */
void TaskbarButton::setProgressMaximum(int progressMaximum) { setProgressRange(std::min(mProgressMinimum, progressMaximum), progressMaximum); }

/*!
 *  Sets the minimum and maximum value of the taskbar button's progress indicator.
 */
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

    updateProgressIndicator();

    if (minChanged)
        emit progressMinimumChanged(mProgressMinimum);
    if (maxChanged)
        emit progressMaximumChanged(mProgressMaximum);
}

/*!
 *  Sets the display state of the taskbar button's progress indicator.
 */
void TaskbarButton::setProgressState(Qx::TaskbarButton::ProgressState progressState)
{
    if(progressState == mProgressState)
        return;

    mProgressState = progressState;
    updateProgressIndicator();
    emit progressStateChanged(mProgressState);
}

/*!
 *  Resets the current value of the taskbar button's progress indicator to the minimum value.
 */
void TaskbarButton::resetProgress() { setProgressValue(mProgressMinimum); }

//-Signals------------------------------------------------------------------------------------------------

/*!
 *  @fn void TaskbarButton::progressValueChanged(int progressValue)
 *
 *  This signal is emitted whenever the taskbar button's current progress value changes.
 */

/*!
 *  @fn void TaskbarButton::progressMinimumChanged(int progressMinimum)
 *
 *  This signal is emitted whenever the taskbar button's minimum progress value changes.
 */

/*!
 *  @fn void TaskbarButton::progressMaximumChanged(int progressMaximum)
 *
 *  This signal is emitted whenever the taskbar button's maximum progress value changes.
 */

/*!
 *  @fn void TaskbarButton::progressStateChanged(Qx::TaskbarButton::ProgressState progressState)
 *
 *  This signal is emitted whenever the taskbar button's progress display state changes.
 */
}
