//! [0]
#include <qx/core/qx-abstracterror.h>

class MyError final : public Qx::AbstractError<"MyError", 1015>
{
private:
    quint16 mErrorValue;
    QString mPrimaryCause;
    
public:
    MyError() :
        mErrorValue(1),
        mPrimaryCause("A bad thing happened.")
    {}
    
private:
    void deriveValue() { return mErrorValue; }
    void derivePrimary() { return mPrimaryCause; }
    //...
}
//! [0]

//! [1]
template<StringLiteral EName, quint16 ECode>
class MyErrorTemplate : public AbstractError<EName, ECode>
{
protected:
    MyErrorTemplate() {}
    
private:
    //...
}
//! [1]

//! [2]
QX_DECLARE_ERROR_TYPE(MyError, "MyError", 1020)
{
    //..
}
//! [2]
