#include "DualshockPad.hpp"
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <memory.h>

#ifdef _WIN32
static inline uint16_t bswap16(uint16_t val) {return _byteswap_ushort(val);}
#elif __GNUC__ && !defined(__FreeBSD__)
static inline uint16_t bswap16(uint16_t val) {return __builtin_bswap16(val); }
#elif !defined(__FreeBSD__)
static inline uint16_t bswap16(uint16_t val) {return __builtin_byteswap(val);}
#endif

#define RAD_TO_DEG (180.0/M_PI)

namespace boo
{
static const uint8_t defaultReport[35] = {
        0x01, 0xff, 0x00, 0xff, 0x00,
        0xff, 0x80, 0x00, 0x00, 0x00,
        0xff, 0x27, 0x10, 0x00, 0x32,
        0xff, 0x27, 0x10, 0x00, 0x32,
        0xff, 0x27, 0x10, 0x00, 0x32,
        0xff, 0x27, 0x10, 0x00, 0x32,
        0x00, 0x00, 0x00, 0x00, 0x00
};

DualshockPad::DualshockPad(DeviceToken* token)
    : DeviceBase(token),
      m_callback(nullptr),
      m_rumbleRequest(EDualshockMotor::None),
      m_rumbleState(EDualshockMotor::None)
{
    memcpy(m_report.buf, defaultReport, 35);
}

DualshockPad::~DualshockPad()
{

}

void DualshockPad::deviceDisconnected()
{
    if (m_callback)
        m_callback->controllerDisconnected();
}

void DualshockPad::initialCycle()
{
    uint8_t setupCommand[5] = {0xF4, 0x42, 0x0c, 0x00, 0x00}; //Tells controller to start sending changes on in pipe
    //if (!sendHIDReport(setupCommand, 4, HIDReportType::Feature, 0xF4))
    //{
    //    deviceError("Unable to send complete packet! Request size %x\n", sizeof(setupCommand));
    //    return;
    //}
    uint8_t btAddr[8];
    //receiveHIDReport(btAddr, sizeof(btAddr), HIDReportType::Feature, 0xF5);
    for (int i = 0; i < 6; i++)
        m_btAddress[5 - i] = btAddr[i + 2]; // Copy into buffer reversed, so it is LSB first
}

void DualshockPad::transferCycle()
{
}

void DualshockPad::finalCycle()
{

}

void DualshockPad::receivedHIDReport(const uint8_t* data, size_t length, HIDReportType tp, uint32_t message)
{
    if (message != 1 || length != 49 || tp != HIDReportType::Input)
        return;
    DualshockPadState state = *reinterpret_cast<const DualshockPadState*>(data);

    for (int i = 0; i < 3; i++)
        state.m_accelerometer[i] = bswap16(state.m_accelerometer[i]);

    state.m_gyrometerZ = bswap16(state.m_gyrometerZ);
    if (m_callback)
        m_callback->controllerUpdate(*this, state);

    if (m_rumbleRequest != m_rumbleState)
    {
        if ((m_rumbleRequest & EDualshockMotor::Left) != EDualshockMotor::None)
        {
            m_report.rumble.leftDuration = m_rumbleDuration[0];
            m_report.rumble.leftForce = m_rumbleIntensity[0];
        }
        else
        {
            m_report.rumble.leftDuration = 0;
            m_report.rumble.leftForce = 0;
        }

        if ((m_rumbleRequest & EDualshockMotor::Right) != EDualshockMotor::None)
        {
            m_report.rumble.rightDuration = m_rumbleDuration[0];
            m_report.rumble.rightOn = true;
        }
        else
        {
            m_report.rumble.rightDuration = 0;
            m_report.rumble.rightOn = false;
        }
        sendHIDReport(m_report.buf, sizeof(m_report), HIDReportType::Output, 0x01);
        m_rumbleState = m_rumbleRequest;
    }
    else
    {
        if (state.m_reserved5[8] & 0x80)
            m_rumbleRequest &= ~EDualshockMotor::Right;
        if (state.m_reserved5[7] & 0x01)
            m_rumbleRequest &= ~EDualshockMotor::Left;
        m_rumbleState = m_rumbleRequest;
        const double zeroG = 511.5; // 1.65/3.3*1023 (1,65V);
        float accXval = -((double)state.m_accelerometer[0] - zeroG);
        float accYval = -((double)state.m_accelerometer[1] - zeroG);
        float accZval = -((double)state.m_accelerometer[2] - zeroG);
        state.accPitch = (atan2(accYval, accZval) + M_PI) * RAD_TO_DEG;
        state.accYaw = (atan2(accXval, accZval) + M_PI) * RAD_TO_DEG;
        state.gyroZ = (state.m_gyrometerZ / 1023.f);
    }
}

} // boo
