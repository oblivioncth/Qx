#ifndef QX_STRINGTRAVERSER_H
#define QX_STRINGTRAVERSER_H

#include <QString>

namespace Qx
{
	
class StringTraverser
{
//-Instance Members----------------------------------------------------------------------------------------------------
private:
    int mIndex;
    QString::const_iterator mIterator;
    QString::const_iterator mEnd;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    StringTraverser(const QString& string);

//-Instance Functions--------------------------------------------------------------------------------------------------
public:
    void advance(int count = 1);

    QChar currentChar();
    int currentIndex();
    QChar lookAhead(int posOffset = 1);
    bool atEnd();
};

}

#endif // QX_STRINGTRAVERSER_H
