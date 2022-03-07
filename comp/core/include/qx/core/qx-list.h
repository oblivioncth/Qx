#ifndef QX_LIST_H
#define QX_LIST_H

// Qt Includes
#include <QList>

#ifdef QT_WIDGETS_LIB // Only enabled for Widgets edition
    #include <QObjectList>
    #include <QWidgetList>
#endif

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

#ifdef QT_WIDGETS_LIB // Only enabled for Widgets edition
    static QWidgetList objectListToWidgetList(QObjectList list);
#endif
};

}

#endif // QX_LIST_H
