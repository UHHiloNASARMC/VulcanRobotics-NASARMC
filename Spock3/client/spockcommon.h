#ifndef SPOCKCOMMON_H
#define SPOCKCOMMON_H

#include <qglobal.h>
#include <qdatastream.h>

enum class ESpockBucketState : quint8
{
    Invalid,
    Scooping,
    Driving,
    Dumping
};

struct SpockMotorStatusData
{
    quint16 current = 0;
    quint8 batteryV = 0;
    quint8 temp = 0;

    void read(QDataStream& s)
    {
        s >> current;
        s >> batteryV;
        s >> temp;
    }

    void write(QDataStream& s) const
    {
        s << current;
        s << batteryV;
        s << temp;
    }
};

struct SpockStatusData
{
    quint64 lastCommandPacket = 0;
    SpockMotorStatusData frontLeftMotorStatus;
    SpockMotorStatusData frontRightMotorStatus;
    SpockMotorStatusData rearLeftMotorStatus;
    SpockMotorStatusData rearRightMotorStatus;
    SpockMotorStatusData armMotorStatus;
    SpockMotorStatusData bucketMotorStatus;
    quint16 armPot = 0;
    quint16 bucketPot = 0;
    ESpockBucketState sensedBucketState = ESpockBucketState::Invalid;

    void read(QDataStream& s)
    {
        s >> lastCommandPacket;
        frontLeftMotorStatus.read(s);
        frontRightMotorStatus.read(s);
        rearLeftMotorStatus.read(s);
        rearRightMotorStatus.read(s);
        armMotorStatus.read(s);
        bucketMotorStatus.read(s);
        s >> armPot;
        s >> bucketPot;
        s >> reinterpret_cast<quint8&>(sensedBucketState);
    }

    void write(QDataStream& s) const
    {
        s << lastCommandPacket;
        frontLeftMotorStatus.write(s);
        frontRightMotorStatus.write(s);
        rearLeftMotorStatus.write(s);
        rearRightMotorStatus.write(s);
        armMotorStatus.write(s);
        bucketMotorStatus.write(s);
        s << armPot;
        s << bucketPot;
        s << reinterpret_cast<const quint8&>(sensedBucketState);
    }
};

struct SpockCommandData
{
    qint16 leftThrottle = 0;
    qint16 rightThrottle = 0;
    ESpockBucketState bucketState = ESpockBucketState::Invalid;
    quint8 bucketOscillate = 0;
    qint16 armAngleBias = 0;
    qint16 linearActuatorBias = 0;
    quint8 cameraEnables = 0;

    void read(QDataStream& s)
    {
        s >> leftThrottle;
        s >> rightThrottle;
        s >> reinterpret_cast<quint8&>(bucketState);
        s >> bucketOscillate;
        s >> armAngleBias;
        s >> linearActuatorBias;
        s >> cameraEnables;
    }

    void write(QDataStream& s) const
    {
        s << leftThrottle;
        s << rightThrottle;
        s << reinterpret_cast<const quint8&>(bucketState);
        s << bucketOscillate;
        s << armAngleBias;
        s << linearActuatorBias;
        s << cameraEnables;
    }
};

#endif // SPOCKCOMMON_H
