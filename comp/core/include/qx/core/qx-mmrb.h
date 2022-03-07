#ifndef QX_MMRB_H
#define QX_MMRB_H

// Qt Includes
#include <QString>

namespace Qx
{

class Mmrb
{
//-Class Variables---------------------------------------------------------------------------------------------
public:
    enum class StringFormat { Full, NoTrailZero, NoTrailRBZero };

//-Member Variables--------------------------------------------------------------------------------------------
private:
    int mMajor;
    int mMinor;
    int mRevision;
    int mBuild;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    Mmrb();
    Mmrb(int major, int minor, int revision, int build);

//-Member Functions--------------------------------------------------------------------------------------------
public:
    bool operator== (const Mmrb& otherMMRB);
    bool operator!= (const Mmrb& otherMMRB);
    bool operator> (const Mmrb& otherMMRB);
    bool operator>= (const Mmrb& otherMMRB);
    bool operator< (const Mmrb& otherMMRB);
    bool operator<= (const Mmrb& otherMMRB);

    bool isNull();
    QString toString(StringFormat format = StringFormat::Full);

    int major();
    int minor();
    int revision();
    int build();

    void setMajor(int major);
    void setMinor(int minor);
    void setRevision(int revision);
    void setBuild(int build);

    void incrementMajor();
    void incrementMinor();
    void incrementRevision();
    void incrementBuild();

private:

//-Class Functions---------------------------------------------------------------------------------------------
public:
    static Mmrb fromString(QString string);
};	

}

#endif // QX_MMRB_H
