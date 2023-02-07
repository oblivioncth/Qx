#ifndef QX_ALGORITHM_H
#define QX_ALGORITHM_H

// Standard Library Includes
#include <stdexcept>
#include <unordered_set>

// Extra-component Includes
#include "qx/utility/qx-concepts.h"

namespace Qx
{
//-Namespace Functions----------------------------------------------------------------------------------------------------
int abs(int n);
unsigned int abs(unsigned int n);
long abs(long n);
unsigned long abs(unsigned long n);
long long abs (long long n);
unsigned long long abs(unsigned long long n);

template<typename T>
    requires std::integral<T>
T length(T start, T end) { return (end - start) + 1; }

template<typename T>
    requires arithmetic<T>
bool isOdd(T num) { return num % 2; }

template<typename T>
    requires arithmetic<T>
bool isEven(T num) { return !isOdd(num); }

template <class InputIt>
    requires std::input_iterator<InputIt> && std::equality_comparable<InputIt>
bool containsDuplicates(InputIt begin, InputIt end)
{
    // Get type that iterator is of
    using T = typename std::iterator_traits<InputIt >::value_type;

    // Place values into unordered set
    std::unordered_set<T> values(begin, end);

    // Count values in original container
    std::size_t size = std::distance(begin,end);

    // Container has duplicates if set size is smaller than the the original container,
    // since sets don't allow for duplicates
    return size != values.size();
}

template<typename T>
    requires arithmetic<T>
T distance(T x, T y) { return std::max(x,y) - std::min(x,y); }

// Thanks to the following for all constrained arithmetic functions:
// https://wiki.sei.cmu.edu/confluence/pages/viewpage.action?pageId=87152052
template <typename T>
	requires std::signed_integral<T>
T constrainedAdd(T a, T b, T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
{
	if((b >= 0) && (a > (max - b)))
		return max; // Overflow
	else if((b < 0) && (a < (min - b)))
		return min; // Underflow
	else
		return a + b;
}

template <typename T>
	requires std::unsigned_integral<T>
T constrainedAdd(T a, T b, T max = std::numeric_limits<T>::max())
{
	if(max - a < b)
		return max; // Overflow
	else
		return a + b;
}

template <typename T>
	requires std::signed_integral<T>
T constrainedSub(T a, T b, T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
{
	if(b >= 0 && a < min + b)
		return min; // Underflow
	else if(b < 0 && a > max + b)
		return max; // Overflow
	else
		return a - b;
}

template <typename T>
	requires std::unsigned_integral<T>
T constrainedSub(T a, T b, T min = 0)
{
	if(a < b)
		return min; // Underflow
	else
		return a - b;
}

template<typename T>
	requires std::signed_integral<T>
T constrainedMult(T a, T b, T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
{
	if(a > 0)
	{
		if(b > 0 && a > (max / b))
			return max; // Overflow
		else if(b < (min / a))
			return min; // Underflow
	}
	else if(a < 0)
	{
		if(b > 0 && a < (min / b))
			return min; // Underflow
		else if(b < (max / a))
			return max; // Overflow
	}

	return a * b;
}

template<typename T>
	requires std::unsigned_integral<T>
T constrainedMult(T a, T b, T max = std::numeric_limits<T>::max())
{
	if(a > max / b)
		return max; // Overflow
	else
		return a * b;
}

template<typename T>
	requires std::signed_integral<T>
T constrainedDiv(T a, T b, T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
{
	if(b == 0)
        qFatal("Divide by zero");

	if((a == std::numeric_limits<T>::min()) && (b == -1))
		return max; // True overflow
	else
	{
		T result = a/b;

		if(result > max)
			return max; // Argument based overflow
		else if(result < min)
			return min; // Argument based underflow
		else
			return result;
	}
}

template<typename T>
	requires std::unsigned_integral<T>
T constrainedDiv(T a, T b, T max = std::numeric_limits<T>::max())
{
	if(b == 0)
        qFatal("Divide by zero");

	T result = a/b;

	if(result > max)
		return max; // Argument based overflow
	else
		return result;
}

template<typename T>
    requires std::integral<T>
T ceilNearestMultiple(T num, T mult)
{
    // Ensure mult is positive
    mult = Qx::abs(mult);

    if(mult == 0)
        return 0;

    if(mult == 1 || mult == num)
        return num;

    if(num < 0)
        return (num / mult) * mult;
    else
    {
        T previousMultiple = (num / mult) * mult;
        return previousMultiple == num ? num : constrainedAdd(previousMultiple, mult);
    }
}

template<typename T>
    requires std::integral<T>
T floorNearestMultiple(T num, T mult)
{
    // Ensure mult is positive
    mult = Qx::abs(mult);

    if(mult == 0)
        return 0;

    if(mult == 1 || mult == num)
        return num;

    if(num > 0)
        return (num / mult) * mult;
    else
    {
        T nextMultiple = (num / mult) * mult;
        return nextMultiple == num ? num : constrainedSub(nextMultiple, mult);
    }
}

template<typename T>
    requires std::integral<T>
T roundToNearestMultiple(T num, T mult)
{
    T above = ceilNearestMultiple(num, mult);
    T below = floorNearestMultiple(num, mult);

    // Return of closest the two directions
    return above - num <= num - below ? above : below;
}

template <typename T>
	requires std::integral<T>
T ceilPowOfTwo(T num)
{
	// Return if num already is power of 2
	if (num && !(num & (num - 1)))
		return num;

	T powOfTwo = 1;

	while(powOfTwo < num)
		powOfTwo <<= 1;

	return powOfTwo;
}

template <typename T>
	requires std::integral<T>
T floorPowOfTwo(T num)
{
	// Return if num already is power of 2
	if (num && !(num & (num - 1)))
		return num;

	// Start with largest possible power of T type can hold
	T powOfTwo = (std::numeric_limits<T>::max() >> 1) + 1;

	while(powOfTwo > num)
		powOfTwo >>= 1;

	return powOfTwo;
}

template <typename T>
	requires std::integral<T>
T roundPowOfTwo(T num)
{
   T above = ceilPowOfTwo(num);
   T below = floorPowOfTwo(num);

   return above - num <= num - below ? above : below;
}

}

#endif // QX_ALGORITHM_H
