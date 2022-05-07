#ifndef QX_TASKBARBUTTON_H
#define QX_TASKBARBUTTON_H

// Qt Includes
#include <QObject>
#include <QWindow>

// Windows Includes
#define NOMINMAX
#include "ShObjIdl_core.h"
#undef NOMINMAX

namespace Qx
{

class TaskbarButton : public QObject
{
//-QObject Macro (Required for all QObject Derived Classes)-----------------------------------------------------------
    Q_OBJECT

//-Class Enums------------------------------------------------------------------------------------------------------
public:
    enum ProgressState {
        Hidden = TBPF_NOPROGRESS,
        Busy = TBPF_INDETERMINATE,
        Normal = TBPF_NORMAL,
        Stopped = TBPF_ERROR,
        Paused = TBPF_PAUSED,
    };
    /*! @cond */
    Q_ENUM(ProgressState); // Register for Meta-Object system
    /*! @endcond */

//-Instance Properties-------------------------------------------------------------------------------------------------------
private:
    // Overlay
    Q_PROPERTY(QIcon overlayIcon READ overlayIcon WRITE setOverlayIcon RESET clearOverlayIcon);
    Q_PROPERTY(QString overlayAccessibleDescription READ overlayAccessibleDescription WRITE setOverlayAccessibleDescription);

    // Window
    Q_PROPERTY(QWindow* window READ window WRITE setWindow);

    // Progress
    Q_PROPERTY(int progressValue READ progressValue WRITE setProgressValue NOTIFY progressValueChanged);
    Q_PROPERTY(int progressMinimum READ progressMinimum WRITE setProgressMinimum NOTIFY progressMinimumChanged);
    Q_PROPERTY(int progressMaximum READ progressMaximum WRITE setProgressMaximum NOTIFY progressMaximumChanged);
    Q_PROPERTY(ProgressState progressState READ progressState WRITE setProgressState NOTIFY progressStateChanged);

//-Instance Members-------------------------------------------------------------------------------------------------
private:
    // Overlay
    QIcon mOverlayIcon;
    QString mOverlayAccessibleDescription;

    // Window
    QWindow* mWindow;

    // Progress
    ITaskbarList4* mTaskbarInterface;
    int mProgressValue;
    int mProgressMinimum;
    int mProgressMaximum;
    ProgressState mProgressState;

//-Constructor-------------------------------------------------------------------------------------------------------
private:
    Q_DISABLE_COPY(TaskbarButton);

public:
    explicit TaskbarButton(QObject *parent = nullptr);

//-Destructor--------------------------------------------------------------------------------------------------------
public:
     ~TaskbarButton();

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    int getNativeIconSize();
    HWND getNativeWindowHandle();

    void updateOverlay();
    void updateProgressIndicator();

public:
    // General
    /*! @cond */
    bool eventFilter(QObject* object, QEvent* event) override;
    /*! @endcond */

    // Overlay
    QIcon overlayIcon() const;
    QString overlayAccessibleDescription() const;

    // Window
    QWindow* window() const;
    void setWindow(QWindow* window);

    // Progress
    int progressValue() const;
    int progressMinimum() const;
    int progressMaximum() const;
    ProgressState progressState() const;

//-Slots------------------------------------------------------------------------------------------------------------
public slots:
    // Overlay
    void setOverlayIcon(const QIcon& icon);
    void setOverlayAccessibleDescription(const QString& description);
    void clearOverlayIcon();

    // Progress
    void setProgressValue(int progressValue);
    void setProgressMinimum(int progressMinimum);
    void setProgressMaximum(int progressMaximum);
    void setProgressRange(int progressMinimum, int progressMaximum);
    void setProgressState(Qx::TaskbarButton::ProgressState progressState);
    void resetProgress();

//-Signals------------------------------------------------------------------------------------------------------------
signals:
    void progressValueChanged(int progressValue);
    void progressMinimumChanged(int progressMinimum);
    void progressMaximumChanged(int progressMaximum);
    void progressStateChanged(Qx::TaskbarButton::ProgressState progressState);
};

#endif // QX_TASHBARBUTTON_H

}
