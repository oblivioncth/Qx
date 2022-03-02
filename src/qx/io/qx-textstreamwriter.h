#ifndef QX_TEXTSTREAMWRITER_H
#define QX_TEXTSTREAMWRITER_H

#include <QTextStream>
#include <QFile>

#include "io/qx-common-io.h"
#include "io/qx-ioopreport.h"

namespace Qx
{

class TextStreamWriter
{
//-Instance Variables------------------------------------------------------------------------------------------------
private:
    QTextStream mStreamWriter;
    QFile* mTargetFile;
    WriteMode mWriteMode;
    WriteOptions mWriteOptions;
    bool mAtLineStart;

//-Constructor-------------------------------------------------------------------------------------------------------
public:
    TextStreamWriter(QFile* file, WriteMode writeMode = Append, WriteOptions writeOptions = NoWriteOptions);

//-Instance Functions------------------------------------------------------------------------------------------------
public:
    IoOpReport openFile();
    IoOpReport writeLine(QString line, bool ensureLineStart = true);
    IoOpReport writeText(QString text);
    void closeFile();
};	

}

#endif // QX_TEXTSTREAMWRITER_H