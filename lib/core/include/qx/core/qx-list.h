#ifndef QX_LIST_H
#define QX_LIST_H

// Qt Includes
#include <QList>

// Extra-component Includes
#include "qx/utility/qx-concepts.h"

namespace Qx
{

class List
{
//-Class Functions----------------------------------------------------------------------------------------------
public:
    template<typename T>
    static QList<T>* subListThatContains(T element, QList<QList<T>*> listOfLists)
    {
        // Returns pointer to the first list that contains "element". Returns nullptr if none
        for(QList<T>* currentList : listOfLists)
           if(currentList->contains(element))
               return currentList;

        return nullptr;
    }

    template<typename T> static QList<T> difference(QList<T>& listA, QList<T>& listB)
    {
        // Difference list to fill
        QList<T> differenceList;

        for(T entry : listA)
        {
            if(!listB.contains(entry))
                differenceList << entry;
        }
        return differenceList;
    }

    template<typename T, typename F>
        requires static_castable_to<F*, T*>
    QList<T*> static_pointer_cast(const QList<F*> fromList)
    {
        QList<T*> toList;
        toList.reserve(fromList.size());
        for(F* item : std::as_const(fromList))
            toList.append(static_cast<T*>(item));

        return toList;
    }

};

}

#endif // QX_LIST_H
