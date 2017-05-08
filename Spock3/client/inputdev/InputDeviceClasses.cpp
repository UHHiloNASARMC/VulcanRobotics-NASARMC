#include "DeviceSignature.hpp"
#include "DualshockPad.hpp"
#include "XInputPad.hpp"

namespace boo
{

const DeviceSignature BOO_DEVICE_SIGS[] =
{
    DEVICE_SIG(DualshockPad, 0x54c, 0x268, DeviceType::HID),
    DEVICE_SIG(XInputPad, 0, 0, DeviceType::XInput),
    DEVICE_SIG_SENTINEL()
};

}
