#pragma once
#include <array>
#include <ll/api/base/StdInt.h>
#include <mc/_HeaderOutputPredefine.h>
#include <mc/deps/raknet/RakNetSocket2.h>
#include <mc/deps/raknet/SystemAddress.h>

namespace RakNet {

struct RNS2RecvStruct {
public:
    std::array<int8, 1600> mData{};
    int                    mBytesRead{};
    RakNet::SystemAddress  mSystemAddress{};
    uint64                 mTimeRead{};
    RakNet::RakNetSocket2* mSocket{};
};

} // namespace RakNet