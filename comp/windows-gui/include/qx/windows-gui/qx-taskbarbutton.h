#ifndef QX_TASKBARBUTTON_H
#define QX_TASKBARBUTTON_H

// Qt Includes
#include <QObject>
#include <QWindow>

// Non-Include Forward Declarations
struct ITaskbarList4;

namespace Qx
{

class TaskbarButton : public QObject
{
//-QObject Macro (Required for all QObject Derived Classes)-----------------------------------------------------------
    Q_OBJECT

//-Class Enums------------------------------------------------------------------------------------------------------
public:
    enum ProgressState {
        Normal,
        Hidden,
        Stopped,
        Paused,
        Busy
    };
    Q_ENUM(ProgressState) // Register for Meta-Object system

//-Instance Properties-------------------------------------------------------------------------------------------------------
private:
    // General
    Q_PROPERTY(QWindow* window READ window WRITE setWindow)

    // Progress
    Q_PROPERTY(int progressValue READ progressValue WRITE setProgressValue NOTIFY progressValueChanged)
    Q_PROPERTY(int progressMinimum READ progressMinimum WRITE setProgressMinimum NOTIFY progressMinimumChanged)
    Q_PROPERTY(int progressMaximum READ progressMaximum WRITE setProgressMaximum NOTIFY progressMaximumChanged)
    Q_PROPERTY(ProgressState progressState READ progressState WRITE setProgressState NOTIFY progressStateChanged)

//-Instance Members-------------------------------------------------------------------------------------------------
private:
    // General
    ITaskbarList4* mTaskbarInterface;
    QWindow* window;

    // Progress
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

//-Class Functions----------------------------------------------------------------------------------------------
private:
    int getNativeProgressState(ProgressState progressState);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void updateProgressValue();
    void updateProgressState();

public:
    int progressValue() const;
    int progressMinimum() const;
    int progressMaximum() const;
    ProgressState progressState() const;

//-Slots------------------------------------------------------------------------------------------------------------
public slots:
    void setProgressValue(int progressValue);
    void setProgressMinimum(int progressMinimum);
    void setProgressMaximum(int progressMaximum);
    void setProgressRange(int progressMinimum, int progressMaximum);
    void setProgressState(Qx::TaskbarButton::ProgressState progressState);
    void resetProgress();

//-Signals------------------------------------------------------------------------------------------------------------
signals:
    void progressValueChanged(int progressValue);
    void progressMinimumChanged(int minimum);
    void progressMaximumChanged(int maximum);
    void progressStateChanged(Qx::TaskbarButton::ProgressState progressState);
};

#endif // QX_TASHBARBUTTON_H

}
