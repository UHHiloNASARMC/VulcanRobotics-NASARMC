#include "DeviceSignature.hpp"
#include "DualshockPad.hpp"

namespace boo
{

const DeviceSignature BOO_DEVICE_SIGS[] =
{
    DEVICE_SIG(DualshockPad, 0x54c, 0x268, DeviceType::HID),
    DEVICE_SIG_SENTINEL()
};

}
