// Unit Includes
#include "qx/core/qx-system.h"

// Qt Includes
#include <QFile>
#include <QDir>
#include <QDirIterator>

// Inner-component Includes
#include <qx/core/qx-regularexpression.h>
#include <qx/core/qx-algorithm.h>

namespace Qx
{

namespace  // Anonymous namespace for effectively private (to this cpp) functions
{
    QString readProcString(quint32 pid, QString subFilePath)
    {
        QFile procFile("/proc/" + QString::number(pid) + subFilePath);
        if(procFile.exists())
        {
            if(procFile.open(QIODevice::ReadOnly))
            {
                QString contents = QString::fromLocal8Bit(procFile.readAll());
                procFile.close();
                return contents;
            }
        }

        return QString();
    }

    QString procNameFromCmdline(quint32 pid)
    {
        QString cmdline = readProcString(pid, "/cmdline");
        if(!cmdline.isEmpty())
        {
            if(cmdline.front() == '-') // Account for login shells
                cmdline.remove(0, 1);

            QString cmdline_arg0 = cmdline.split('\0').at(0);
            QFileInfo procPathInfo(cmdline_arg0);

            return procPathInfo.baseName();
        }

        return QString();
    }

    QString procNameFromStat(quint32 pid)
    {
        QString stat = readProcString(pid, "/stat");
        if(!stat.isEmpty())
        {
            // Single out name
            qsizetype nameStartMarker = stat.indexOf('(');
            qsizetype nameEndMarker = stat.lastIndexOf(')');

            if(nameStartMarker != -1 && nameEndMarker != -1)
            {
                qsizetype nameStart = nameStartMarker + 1;
                qsizetype nameEnd = nameEndMarker - 1;
                QString name = stat.sliced(nameStart, Qx::lengthOfRange(nameStart, nameEnd));

                // Name should be limited to 15 char (excluding '\0')
                if(name.size() <= 15)
                    return name;
            }
        }

        return QString();
    }

    QString procNameFromExe(quint32 pid)
    {
        QFileInfo exeSymlink("/proc/" + QString::number(pid) + "/exe");
        if(exeSymlink.exists() && exeSymlink.isSymLink())
        {
            QFileInfo exe(exeSymlink.symLinkTarget());
            return exe.baseName();
        }

        return QString();
    }
}

//-Namespace Functions-------------------------------------------------------------------------------------------------------------
quint32 processId(QString processName)
{
    // Iterate over /proc sub-directories
    QDirIterator dit("/proc", QDir::Dirs | QDir::NoDotAndDotDot);
    while(dit.hasNext())
    {
        QString currentDirName = dit.nextFileInfo().fileName();

        // Ignore non-process specific folders
        if(!currentDirName.contains(Qx::RegularExpression::NUMBERS_ONLY))
            continue;

        // Note process ID
        quint32 pid = currentDirName.toInt();

        // Check for all match methods
        if(processName == readProcString(pid, "/comm") ||
           processName == procNameFromCmdline(pid) ||
           processName == procNameFromStat(pid) ||
           processName == procNameFromExe(pid))
            return pid;
    }

    // No match
    return 0;
}


QString processName(quint32 processId)
{
    /* The 'comm' field of /stat is the most canon process name since that respects Linux's limit of 15 chars,
     * so a user should never see a larger process name that they'd be looking for. This would be different
     * in a function described as finding the PID of a process based on its executable name, but this is
     * based specifically on process name.
     */
    return procNameFromStat(processId);
}

}
