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

}

}
