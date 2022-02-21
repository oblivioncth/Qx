#include "qx-mmrb.h"

#include <QStringList>

#include "core/qx-string.h"

namespace Qx
{

//===============================================================================================================
// Mmrb
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------
Mmrb::Mmrb() :
    mMajor(-1),
    mMinor(-1),
    mRevision(-1),
    mBuild(-1)
{}

Mmrb::Mmrb(int major, int minor, int revision, int build) :
    mMajor(major),
    mMinor(minor),
    mRevision(revision),
    mBuild(build)
{}

//-Member Functions--------------------------------------------------------------------------------------------
//Public:
bool Mmrb::operator== (const Mmrb& otherMmrb)
{
    return mMajor == otherMmrb.mMajor && mMinor == otherMmrb.mMinor && mRevision == otherMmrb.mRevision && mBuild == otherMmrb.mBuild;
}
bool Mmrb::operator!= (const Mmrb& otherMmrb) { return !(*this == otherMmrb); }
bool Mmrb::operator> (const Mmrb& otherMmrb)
{
    if(mMajor == otherMmrb.mMajor)
    {
        if(mMinor == otherMmrb.mMinor)
        {
            if(mRevision == otherMmrb.mRevision)
                return mBuild > otherMmrb.mBuild;
            else
                return mRevision > otherMmrb.mRevision;
        }
        else
            return mMinor > otherMmrb.mMinor;
    }
    else
        return mMajor > otherMmrb.mMajor;
}
bool Mmrb::operator>= (const Mmrb& otherMmrb) { return *this == otherMmrb || *this > otherMmrb; }
bool Mmrb::operator< (const Mmrb& otherMmrb) { return !(*this >= otherMmrb); }
bool Mmrb::operator<= (const Mmrb& otherMmrb) { return !(*this > otherMmrb); }

bool Mmrb::isNull() { return mMajor == -1 && mMinor == -1 && mRevision == -1 && mBuild == -1; }

QString Mmrb::toString(Mmrb::StringFormat format)
{
    QString workingString = QString::number(mMajor);

    if(mMinor != 0 || mRevision != 0 || mBuild != 0 || format != StringFormat::NoTrailZero)
        workingString += "." + QString::number(mMinor);

    if(mRevision != 0 || mBuild != 0 || format == StringFormat::Full)
        workingString += "." + QString::number(mRevision);

    if(mBuild != 0 || format == StringFormat::Full)
        workingString += "." + QString::number(mBuild);

    return workingString;
}

int Mmrb::major() { return mMajor; }
int Mmrb::minor() { return mMinor; }
int Mmrb::revision() { return mRevision; }
int Mmrb::build() { return mBuild; }

void Mmrb::setMajor(int major) { mMajor = major; }
void Mmrb::setMinor(int minor) { mMinor = minor; }
void Mmrb::setRevision(int revision) { mRevision = revision; }
void Mmrb::setBuild(int build) { mBuild = build; }

void Mmrb::incrementMajor() { mMajor++; }
void Mmrb::incrementMinor() { mMinor++; }
void Mmrb::incrementRevision() { mRevision++; }
void Mmrb::incrementBuild() { mBuild++; }

//-Class Functions---------------------------------------------------------------------------------------------
//Public:
Mmrb Mmrb::fromString(QString string)
{
    // Check for valid string
    if(!String::isValidMmrb(string))
        return Mmrb();

    int versions[] = {0, 0, 0, 0}; // Above check prevents going OOB with this

    QStringList segments = string.split('.');

    for(int i = 0; i < segments.size(); i++)
        versions[i] = segments.at(i).toInt();

    return Mmrb(versions[0], versions[1], versions[2], versions[3]);
}	

}
