#ifndef QX_PROCCESSBIDER_H
#define QX_PROCCESSBIDER_H

// Shared Lib Support
#include "qx/core/qx_core_export.h"

// Qt Includes
#include <QObject>

// Inter-component Includes
#include <qx/core/qx-abstracterror.h>

using namespace std::chrono_literals;

namespace Qx
{

class QX_CORE_EXPORT ProcessBiderError final : public AbstractError<"Qx::ProcessBiderError", 6>
{
    friend class ProcessBider;
//-Class Enums-------------------------------------------------------------
public:
    enum Type
    {
        NoError,
        FailedToHook,
        FailedToClose
    };

//-Class Variables-------------------------------------------------------------
private:
    static inline const QHash<Type, QString> ERR_STRINGS{
        {NoError, u""_s},
        {FailedToHook, u"Could not hook the process in order to bide on it."_s},
        {FailedToClose, u"Could not close the bided process."_s}
    };

//-Instance Variables-------------------------------------------------------------
private:
    Type mType;
    QString mProcessName;

//-Constructor-------------------------------------------------------------
private:
    ProcessBiderError(Type t, const QString& pn);

//-Instance Functions-------------------------------------------------------------
private:
    quint32 deriveValue() const override;
    QString derivePrimary() const override;
    QString deriveSecondary() const override;

public:
    bool isValid() const;
    Type type() const;
    QString processName() const;
};

class QX_CORE_EXPORT ProcessBider : public QObject
{
    friend class ProcessBiderManager;
    Q_OBJECT
//-Class Types----------------------------------------------------------------------------------------------
public:
    enum ResultType { Fail, Expired, Abandoned };

//-Instance Members------------------------------------------------------------------------------------------
private:
    // Data
    QString mName;
    std::chrono::milliseconds mGrace;
#ifdef __linux__
    std::chrono::milliseconds mPollRate;
#endif
    bool mInitialGrace;

    // Functional
    bool mBiding;

//-Constructor----------------------------------------------------------------------------------------------
public:
    explicit ProcessBider(QObject* parent = nullptr, const QString& processName = {});

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    bool isBiding() const;
    QString processName() const;
    std::chrono::milliseconds respawnGrace() const;
    bool initialGrace() const;

    void setProcessName(const QString& name);
    void setRespawnGrace(std::chrono::milliseconds grace);
    void setInitialGrace(bool initialGrace);

#ifdef __linux__
    std::chrono::milliseconds pollRate() const;
    void setPollRate(std::chrono::milliseconds rate);
#endif

//-Slots------------------------------------------------------------------------------------------------------------
private slots:
    void handleResultReady(ResultType result);
    void handleCloseFailure();

public slots:
    void start();
    void stop();
    void closeProcess(std::chrono::milliseconds timeout = 1000ms, bool force = false);

//-Signals------------------------------------------------------------------------------------------------------------
signals:
    void started();
    void established();
    void graceStarted();
    void processStopped();
    void processClosing();
    void stopped();
    void errorOccurred(ProcessBiderError error);
    void finished(ResultType result);

/*! @cond */
    // Temporary until using PIMPL or changing implementation otherwise
    void __startClose(std::chrono::milliseconds timeout, bool force);
/*! @endcond */
};

}

#endif // QX_PROCCESSBIDER_H
