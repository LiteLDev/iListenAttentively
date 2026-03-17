#include <ll/api/base/StdInt.h>
#include <mc/deps/nether_net/p2p/NetworkID.h>

namespace NetherNet::P2P {

NetworkID::NetworkID() : mUnk9df456() { mUnk9df456.as<uint64>() = 0; }

NetworkID::NetworkID(NetworkID const& other) : mUnk9df456() { mUnk9df456.as<uint64>() = other.mUnk9df456.as<uint64>(); }

NetworkID& NetworkID::operator=(NetworkID const& other) {
    mUnk9df456.as<uint64>() = other.mUnk9df456.as<uint64>();
    return *this;
}

} // namespace NetherNet::P2P