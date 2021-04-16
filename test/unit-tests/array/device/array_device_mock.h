#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/device/array_device.h"

namespace pos
{
class MockArrayDevice : public ArrayDevice
{
public:
    using ArrayDevice::ArrayDevice;
    MOCK_METHOD(ArrayDeviceState, GetState, (), (override));
    MOCK_METHOD(void, SetState, (ArrayDeviceState state), (override));
    MOCK_METHOD(UblockSharedPtr, GetUblock, (), (override));
    MOCK_METHOD(void, SetUblock, (UblockSharedPtr uBlock), (override));
    MOCK_METHOD(bool, IsRebuilding, (), (override));
    MOCK_METHOD(void, SetRebuild, (bool rebuild), (override));
};

} // namespace pos