//! [0]
#include <QList>
#include <qx/core/qx-abstracterror.h>
#include <qx/core/qx-error.h>
#include <qx/core/qx-iostream.h>

// Realistically these custom error types would be fleshed out much more
class BadError final : public Qx::AbstractError<"BadError", 1000>
{
public:
    enum Type
    {
        NoError = 0,
        Bad = 1,
        Worse = 2
    };

private:
    Type mValue;

public:
    BadError() :
        mValue(NoError)
    {}
    BadError(Type type) :
        mValue(type)
    {}

private:
    quint32 deriveValue() const override { return mValue; }
    QString derivePrimary() const override { return "Bad error occurred."; }
    QString deriveSecondary() const override { return QString("Error type %1").arg(mValue); }

public:
    bool isProblem() { return mValue != NoError; }
};

class BigBadError final : public Qx::AbstractError<"BigBadError", 1001>
{
public:
    enum Type
    {
        NoError = 0,
        Big = 1,
        Bigger = 2
    };

private:
    Type mValue;

public:
    BigBadError() :
        mValue(NoError)
    {}
    BigBadError(Type type) :
        mValue(type)
    {}

private:
    quint32 deriveValue() const override { return mValue; }
    QString derivePrimary() const override { return "Big bad error occurred."; }
    QString deriveSecondary() const override { return QString("Error type %1").arg(mValue); }

public:
    bool isProblem() { return mValue != NoError; }
};

BadError procedure()
{
    //...
    bool problem = true; // For demonstration purposes
    if(problem)
        return BadError(BadError::Bad);
    //...

    return BadError();
}

BigBadError bigProcedure()
{
    //...
    bool hugeProblem = true; // For demonstration purposes
    if(hugeProblem)
        return BigBadError(BigBadError::Bigger);
    //...

    return BigBadError();
}

Qx::Error doStuff()
{
    //...
    BadError be = procedure();
    if(be.isProblem())
        return be;

    BigBadError bbe = bigProcedure();
    if(bbe.isProblem())
        return bbe;
    //...

    return Qx::Error();
}

int main()
{
    //...
    QList<Qx::Error> errors;
    errors << BadError(BadError::NoError);
    errors << doStuff();
    //...

    for(const Qx::Error& e : errors)
    {
        // Print first valid error
        if(e.isValid())
        {
            Qx::cout << "First Error:\n" << e;
            break;
        }
    }

    //...
    return 0;
}
//! [0]

//! [1]
#include <qx/core/qx-abstracterror.h>
#include <qx/core/qx-error.h>
#include <qx/core/qx-iostream.h>
#include <QSqlError>

class QSqlErrorAdapter final : public Qx::AbstractError<"QSqlError", 1200>
{
private:
    const QSqlError& mErrorRef;

public:
    QSqlErrorAdapter(const QSqlError& e) :
        mErrorRef(e)
    {}
    QSqlErrorAdapter(QSqlErrorAdapter&&) = delete;
    QSqlErrorAdapter(const QSqlErrorAdapter&) = delete;

private:
    quint32 deriveValue() const override { return mErrorRef.type(); }
    QString derivePrimary() const override { return "An SQL error occurred."; }
    QString deriveSecondary() const override { return mErrorRef.text(); }
};
QX_DECLARE_ERROR_ADAPTATION(QSqlError, QSqlErrorAdapter);

int main(int argc, char *argv[])
{
    // Example QSqlError
    QSqlError e("Faulty Driver", "Issue loading driver", QSqlError::ConnectionError);

    // Print via Qx::Error overload of QTextStream::operator<<()
    Qx::cout << e;
}
// Prints:
/*
 *	( ERROR ) 0x0204B000000001 
 *  An SQL error occurred.
 *  Issue loading driver Faulty Driver
 */
//! [1]

//! [2]
Qx::GenericError ge;
ge.setSeverity(Qx::Warning);
ge.setValue(50);
ge.setCaption("Caption");
ge.setPrimary("Generic Error");
ge.setSecondary("There was an Error");
ge.setDetailed("- Issue 1\n- Issue2\n- Issue3");

QTextStream ts;
ts << Qx::Error(ge);

// Prints:
/*
 *	(WARNING) 0x00000032 Caption 
 *  Generic Error
 *  There was an Error
 *
 *	Details:
 *  --------
 *  - Issue 1
 *  - Issue 2
 *  - Issue 3
 */
//! [2]
