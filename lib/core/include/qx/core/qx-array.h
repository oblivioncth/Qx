#ifndef QX_ARRAY_H
#define QX_ARRAY_H

// Qt Includes
#include <QHash>

// Extra-component Includes
#include "qx/utility/qx-concepts.h"

// TODO: These functions are pretty underwhelming, see if more flexible ones that make use
// of range/iterator based functions from std can be made instead.

namespace Qx
{
	
class Array
{
//-Class Functions----------------------------------------------------------------------------------------------
public:
    template <typename T, int N>
    static constexpr int constDim(const T(&)[N]) { return N; } // Allows using the size of a const array at runtime

    template <typename T, int N>
    static int indexOf(const T(&array) [N], const T query)
    {
        for(int i = 0; i < N; i++)
            if(array[i] == query)
                return i;

        return -1;
    }

    template<typename T, int N>
        requires arithmetic<T>
    static T maxOf(const T(&array) [N])
    {
       T max = array[0];

       for(int i = 1; i < N; i++)
           if(array[i] > max)
               max = array[i];

       return max;
    }

    template<typename T, int N>
        requires arithmetic<T>
    static T minOf(const T(&array) [N])
    {
        T min = array[0];

        for(int i = 1; i < N; i++)
            if(array[i] < min)
                min = array[i];

        return min;
    }

    template<typename T, int N>
    static T mostFrequent(const T(&array) [N])
    {
        // Load all array elements into a hash
        QHash<T,int> hash;
        for(int i = 0; i < N; i++)
            hash[array[i]]++;

        // Determine greatest frequency
        int maxFreq = 0;
        T maxFreqVal = array[0]; // Assume first value is most frequent to start
        QHashIterator<T,int> i(hash);

        while(i.hasNext())
        {
            i.next();
            if(maxFreq < i.value())
            {
                maxFreqVal = i.key();
                maxFreq = i.value();
            }
        }

         return maxFreqVal;
    }
};

}

#endif // QX_ARRAY_H
