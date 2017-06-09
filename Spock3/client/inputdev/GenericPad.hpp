#ifndef CGENERICPAD_HPP
#define CGENERICPAD_HPP

#include "DeviceBase.hpp"

namespace boo
{

class GenericPad;
struct IGenericPadCallback
{
    virtual void controllerDisconnected() {}
    virtual void axisMove(GenericPad&, unsigned /*usage*/,
                          const char* /*name*/, int16_t /*value*/) {}
    virtual void buttonPress(GenericPad&, unsigned /*usage*/,
                             const char* /*name*/, bool /*pressed*/) {}
};

class GenericPad final : public DeviceBase
{
    IGenericPadCallback* m_callback = nullptr;
public:
    GenericPad(DeviceToken* token);
    ~GenericPad();

    void setCallback(IGenericPadCallback* cb) { m_callback = cb; }

    void deviceDisconnected();
    void receivedHIDValueChange(HIDValueType valType, unsigned usage,
                                const char* name, int value);
};

}

#endif // CGENERICPAD_HPP
