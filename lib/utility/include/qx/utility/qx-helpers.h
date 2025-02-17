#ifndef QX_HELPERS_H
#define QX_HELPERS_H

// Standard Library Includes
#include <type_traits>
#include <utility>

// Qt Includes
#include <QMetaObject>
#include <QObject>

// Inner-component Includes
#include "qx/utility/qx-typetraits.h"
#include "qx/utility/qx-concepts.h"

namespace Qx
{

class ScopedConnection
{
    Q_DISABLE_COPY(ScopedConnection);
//-Instance Variables------------------------------------------------------------------------------------------
private:
    QMetaObject::Connection mConnection;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    inline ScopedConnection(const QMetaObject::Connection& connection) : mConnection(connection) {}
    inline ScopedConnection(ScopedConnection&& other) = default;

//-Destructor-------------------------------------------------------------------------------------------------
public:
    inline ~ScopedConnection() { if(mConnection) QObject::disconnect(mConnection); }

//-Operators--------------------------------------------------------------------------------------------------
public:
    inline ScopedConnection& operator=(ScopedConnection&& other) = default;
    inline operator bool() const { return static_cast<bool>(mConnection); }
};

//Namespace Functions-----------------------------------------------------------------------------------------
template<typename PointerToMemberFunction, typename Functor>
ScopedConnection scopedConnect(const QObject* sender, PointerToMemberFunction signal, Functor&& functor)
{
    return QObject::connect(sender, std::forward<PointerToMemberFunction>(signal), std::forward<Functor>(functor));
}

//TODO: Might want to move this to a container tools specific header (along with some other todos related to them)
template<typename T>
using container_arrow_result = std::conditional_t<defines_member_ptr<T>, T&,
                               std::conditional_t<std::is_class_v<target_type<T>>, target_type<T>*, void>>;

template<typename T>
concept arrowable_container_type = defines_member_ptr<T> || std::is_class_v<target_type<T>>;

template<arrowable_container_type T>
container_arrow_result<T> container_arrow_operator(T& data)
{
    if constexpr(defines_member_ptr<T> || std::is_pointer_v<T>)
        return data;
    else
        return &data;
}

}

//Non-namespace Structs----------------------------------------------------------
/* TODO: Figure out how to constrain this to only accept functors, issue is at least as of C++20
 * there doesnt seem to be a way to check if a type has an arbitrary number of operator() overloads
 * with an arbitrary number of arguments.
 */
template<typename... Functors>
struct qxFuncAggregate : Functors... {
    using Functors::operator()...;
};

/* Explicit deduction guide. Shouldn't be needed as of C++20, but some compilers are late to the party.
 * We'd like to check for #if __cpp_deduction_guides < 201907 to see if the compiler has the feature,
 * but it seems that GCC 10.x still won't compile without this even though it reports 201907
 * implementation. This might have been an oversight, or have to due with P2082R1
 * (Fixing CTAD for aggregates) which wasn't implemented in GCC until 11.x
 */
/*! @cond */
template<class... Ts>
qxFuncAggregate(Ts...) -> qxFuncAggregate<Ts...>;
/*! @endcond */

//Non-namespace Functions----------------------------------------------------------
template <typename T>
const T qxAsConst(T&& t) { return std::move(t); }

template <typename T>
typename std::add_const<T>::type qxAsConst(T& t) { return std::as_const(t); }

template <typename T>
void qxDelete(T*& pointer)
{
    delete pointer;
    pointer = nullptr;
}

#endif // QX_HELPERS_H
