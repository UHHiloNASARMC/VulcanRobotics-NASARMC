#include "GenericPad.hpp"
#include "DeviceToken.hpp"

namespace boo
{

GenericPad::GenericPad(DeviceToken* token)
 : DeviceBase(token)
{

}

GenericPad::~GenericPad()
{

}

void GenericPad::deviceDisconnected()
{
    if (m_callback)
        m_callback->controllerDisconnected();
}

void GenericPad::receivedHIDValueChange(HIDValueType valType, unsigned usage,
                                        const char* name, int value)
{
    if (valType == HIDValueType::Axis)
    {
        if (m_callback)
            m_callback->axisMove(*this, usage, name, value);
    }
    else if (valType == HIDValueType::Button)
    {
        if (m_callback)
            m_callback->buttonPress(*this, usage, name, value);
    }
}

}
