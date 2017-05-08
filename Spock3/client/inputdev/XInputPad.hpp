#ifndef XINPUTPAD_HPP
#define XINPUTPAD_HPP

#include "DeviceBase.hpp"

namespace boo
{

struct XInputPadState
{
    uint16_t wButtons;
    uint8_t bLeftTrigger;
    uint8_t bRightTrigger;
    int16_t sThumbLX;
    int16_t sThumbLY;
    int16_t sThumbRX;
    int16_t sThumbRY;
};

class XInputPad;
struct IXInputPadCallback
{
    virtual void controllerDisconnected() {}
    virtual void controllerUpdate(const XInputPadState&) {}
};

class XInputPad final : public DeviceBase
{
    friend class HIDListenerWinUSB;
    IXInputPadCallback* m_callback;
public:
    XInputPad(DeviceToken* token) : DeviceBase(token) {}
    void setCallback(IXInputPadCallback* cb) { m_callback = cb; }
    void deviceDisconnected()
    {
        if (m_callback)
            m_callback->controllerDisconnected();
    }
};

}

#endif // XINPUTPAD_HPP
