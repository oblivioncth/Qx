#include "qx-stringtraverser.h"

#include <stdexcept>

namespace Qx
{

//===============================================================================================================
// StringTraverser
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------------
//Public:
StringTraverser::StringTraverser(const QString& string) :
    mIndex(0),
    mIterator(string.constBegin()),
    mEnd(string.constEnd())
{}

//-Instance Functions--------------------------------------------------------------------------------------------------
//Public:
void StringTraverser::advance(int count)
{
    if(!atEnd())
    {
        assert(count > 0);
        mIterator += count;
        mIndex += count;
    }
    else
        throw std::out_of_range(QString("Premature end of string at " + QString::number(mIndex)).toUtf8().data());
}

QChar StringTraverser::currentChar() { return *mIterator; }
int StringTraverser::currentIndex() { return mIndex; }
QChar StringTraverser::lookAhead(int posOffset) { return *(mIterator + posOffset); }
bool StringTraverser::atEnd() { return mIterator == mEnd; }	

}
