#ifndef QX_ALGORITHM_H
#define QX_ALGORITHM_H

#include <stdexcept>

#include "utility/qx-concepts.h"

namespace Qx
{
//-Namespace Functions----------------------------------------------------------------------------------------------------
template<typename T>
    requires std::integral<T>
T lengthOfRange(T start, T end) { return (end - start) + 1; }

template<typename T>
    requires arithmetic<T>
static bool isOdd(T num) { return num % 2; }

template<typename T>
    requires arithmetic<T>
static bool isEven(T num) { return !isOdd(num); }

template <typename T>
	requires std::signed_integral<T>
static T constrainedAdd(T a, T b, T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
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
static T constrainedAdd(T a, T b, T max = std::numeric_limits<T>::max())
{
	if(max - a < b)
		return max; // Overflow
	else
		return a + b;
}

template <typename T>
	requires std::signed_integral<T>
static T constrainedSub(T a, T b, T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
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
static T constrainedSub(T a, T b, T min = 0)
{
	if(a < b)
		return min; // Underflow
	else
		return a - b;
}

template<typename T>
	requires std::signed_integral<T>
static T constrainedMult(T a, T b, T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
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
static T constrainedMult(T a, T b, T max = std::numeric_limits<T>::max())
{
	if(a > max / b)
		return max; // Overflow
	else
		return a * b;
}

template<typename T>
	requires std::signed_integral<T>
static T constrainedDiv(T a, T b, T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
{
	if(b == 0)
		throw std::logic_error("Divide by zero");

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
static T constrainedDiv(T a, T b, T max = std::numeric_limits<T>::max())
{
	if(b == 0)
		throw std::logic_error("Divide by zero");

	T result = a/b;

	if(result > max)
		return max; // Argument based overflow
	else
		return result;
}

template<typename T>
	requires std::integral<T>
static T roundToNearestMultiple(T num, T mult)
{
	// Ignore negative multiples
	mult = std::abs(mult);

	if(mult == 0)
		return 0;

	if(mult == 1)
		return num;

	T towardsZero = (num / mult) * mult;
	T awayFromZero = num < 0 ? constrainedSub(towardsZero, mult) : constrainedAdd(towardsZero, mult);

	// Return of closest the two directions
	return (abs(num) - abs(towardsZero) >= abs(awayFromZero) - abs(num))? awayFromZero : towardsZero;
}

template <typename T>
	requires std::integral<T>
static T ceilPowOfTwo(T num)
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
static T floorPowOfTwo(T num)
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
static T roundPowOfTwo(T num)
{
   T above = ceilPowOfTwo(num);
   T below = floorPowOfTwo(num);

   return above - num <= num - below ? above : below;
}

}

#endif // QX_ALGORITHM_H
