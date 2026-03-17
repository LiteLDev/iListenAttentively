#include <ll/api/base/StdInt.h>
#include <mc/deps/nether_net/realms/NetworkID.h>

namespace NetherNet::Realms {

NetworkID::NetworkID() : mUnk81076b() { mUnk81076b.as<uint64>() = 0; }

NetworkID::NetworkID(NetworkID const& other) : mUnk81076b() { mUnk81076b.as<uint64>() = other.mUnk81076b.as<uint64>(); }

NetworkID& NetworkID::operator=(NetworkID const& other) {
    mUnk81076b.as<uint64>() = other.mUnk81076b.as<uint64>();
    return *this;
}

} // namespace NetherNet::Realms