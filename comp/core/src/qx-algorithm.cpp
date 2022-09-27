// Unit Includes
#include "qx/core/qx-algorithm.h"

// Standard Library Includes
#include <cmath>

/*!
 *  @file qx-algorithm.h
 *  @ingroup qx-core
 *
 *  @brief The qx-algorithm header file provides various mathematical/algorithmic functions.
 */

namespace Qx
{
//-Namespace Functions----------------------------------------------------------------------------------------------------
/*!
 *  Returns the absolute value of @a n; equivalent to @c std::abs(n) .
 */
int abs(int n) { return std::abs(n); }

/*!
 *  @overload
 *
 *  Returns the absolute value of @a n; equivalent to @c n.
 *
 *  This overload allows @c abs to be used in templates without the need to specialize solely because of signedness
 */
unsigned int abs(unsigned int n) { return n; }

/*!
 *  @overload
 *  @copydoc abs(int n)
 */
long abs(long n) { return std::abs(n); }

/*!
 *  @copydoc abs(unsigned int n)
 */
unsigned long abs(unsigned long n) { return n; }

/*!
 *  @overload
 *  @copydoc abs(int n)
 */
long long abs (long long n) { return std::abs(n); }

/*!
 *  @copydoc abs(unsigned int n)
 */
unsigned long long abs(unsigned long long n) { return n; }

/*!
 *  @fn template<typename T> requires std::integral<T> T length(T start, T end)
 *
 *  Computes the length of, or alternatively the number of elements in, the range [@a start,  @a end].
 *
 *  This is equivalent to: <tt>(end - start) + 1</tt>.
 */

/*!
 *  @fn template<typename T> requires arithmetic<T> static bool isOdd(T num)
 *
 *  Returns @c true if @a num is odd; otherwise returns @c false.
 */

/*!
 *  @fn template<typename T> requires arithmetic<T> static bool isEven(T num)
 *
 *  Returns @c true if there are duplicate elements in the range [@a first, @a last); otherwise returns @c false.
 */

/*!
 *  @fn template <class InputIt> requires std::input_iterator<InputIt> && std::equality_comparable<InputIt> bool containsDuplicates(InputIt begin, InputIt end)
 *
 *  Returns @c true if @a num is even; otherwise returns @c false.
 */

/*!
 *  @fn template<typename T> requires arithmetic<T> static T distance(T x, T y)
 *
 *  Returns the absolute distance between @a x and @a y.
 */

/*!
 *  @fn template <typename T> requires std::signed_integral<T> static T constrainedAdd(T a, T b, T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
 *
 *  Returns the result of adding signed integers @a a and @a b, bounded between @a min and @a max inclusive.
 *
 *  The default arguments of this function make it useful for performing addition that is safe from underflow and overflow.
 */

/*!
 *  @fn template <typename T> requires std::unsigned_integral<T> static T constrainedAdd(T a, T b, T max = std::numeric_limits<T>::max())
 *  Returns the result of adding unsigned integers @a a and @a b, bounded between 0 and @a max inclusive.
 *
 *  The default arguments of this function make it useful for performing addition that is safe from overflow.
 */

/*!
 *  @fn template <typename T> requires std::signed_integral<T> static T constrainedSub(T a, T b, T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
 *
 *  Returns the result of subtracting signed integers @a a and @a b, bounded between @a min and @a max inclusive.
 *
 *  The default arguments of this function make it useful for performing subtraction that is safe from underflow and overflow.
 */

/*!
 *  @fn template <typename T> requires std::unsigned_integral<T>static T constrainedSub(T a, T b, T min = 0)
 *
 *  Returns the result of subtracting unsigned integers @a a and @a b, bounded between @a min and @c "std::numeric_limits<T>::max()" inclusive.
 *
 *  The default arguments of this function make it useful for performing subtraction that is safe from underflow.
 */

/*!
 *  @fn template<typename T> requires std::signed_integral<T> static T constrainedMult(T a, T b, T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
 *
 *  Returns the result of multiplying signed integers @a a and @a b, bounded between @a min and @a max inclusive.
 *
 *  The default arguments of this function make it useful for performing multiplication that is safe from underflow and overflow.
 */

/*!
 *  @fn template<typename T> requires std::unsigned_integral<T> static T constrainedMult(T a, T b, T max = std::numeric_limits<T>::max())
 *
 *  Returns the result of multiplying unsigned integers @a a and @a b, bounded between 0 and @a max inclusive.
 *
 *  The default arguments of this function make it useful for performing multiplication that is safe from overflow.
 */

/*!
 *  @fn template<typename T> requires std::signed_integral<T> static T constrainedDiv(T a, T b, T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
 *
 *  Returns the result of dividing signed integer @a a by signed integer @a b, bounded between @a min and @a max inclusive.
 *
 *  The default arguments of this function make it useful for performing division that is safe from underflow and overflow.
 */

/*!
 *  @fn template<typename T> requires std::unsigned_integral<T> static T constrainedDiv(T a, T b, T max = std::numeric_limits<T>::max())
 *
 *  Returns the result of dividing unsigned integer @a a by signed integer @a b, bounded between 0 and @a max inclusive.
 */

/*!
 *  @fn template<typename T> requires std::integral<T> static T ceilNearestMultiple(T num, T mult)
 *
 *  Returns the next (i.e. higher) multiple of @a mult after @a num, or @a num if it is already
 *  a multiple of @a mult.
 *
 *  The sign of the result will always be the same sign as @a num, regardless of the sign of @a mult.
 */

/*!
 *  @fn template<typename T> requires std::integral<T> static T floorNearestMultiple(T num, T mult)
 *
 *  Returns the previous (i.e. lower) multiple of @a mult after @a num, or @a num if it is already
 *  a multiple of @a mult.
 *
 *  The sign of the result will always be the same sign as @a num, regardless of the sign of @a mult.
 */

/*!
 *  @fn template<typename T> requires std::integral<T> static T roundToNearestMultiple(T num, T mult)
 *
 *  Returns the multiple of @a mult that @a num is closest to.
 *
 *  The sign of the result will always be the same sign as @a num, regardless of the sign of @a mult.
 */

/*!
 *  @fn template <typename T> requires std::integral<T> static T ceilPowOfTwo(T num)
 *
 *  Returns the next (i.e. higher) power of two after @a num, or @a num if it is
 *  already a power of two.
 */

/*!
 *  @fn template <typename T> requires std::integral<T> static T floorPowOfTwo(T num)
 *
 *  Returns the previous (i.e. lower) power of two before @a num, or @a num if it is
 *  already a power of two.
 */

/*!
 *  @fn template <typename T> requires std::integral<T> static T roundPowOfTwo(T num)
 *
 *  Returns the power of two that is closest to @a num.
 */
}
