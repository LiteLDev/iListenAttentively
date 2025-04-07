#include "ila/iListenAttentively.h"
#include "ila/base/Gloabl.h"
#include <ll/api/mod/RegisterHelper.h>
#include <ll/api/service/Bedrock.h>
#include <ll/api/thread/ServerThreadExecutor.h>
#include <mc/network/NetworkConnection.h>
#include <mc/network/NetworkSystem.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/dimension/Dimension.h>
#include <mc/world/level/dimension/VanillaDimensions.h>

namespace ila
{

iListenAttentively& iListenAttentively::getInstance()
{
    static iListenAttentively instance;
    return instance;
}

bool iListenAttentively::load() { return true; }

bool iListenAttentively::enable() { return true; }

bool iListenAttentively::disable() { return true; }

void nextTick(std::function<void()> const& func)
{
    ll::thread::ServerThreadExecutor::getDefault().executeAfter(
        [func { std::move(func) }]() -> void { func(); },
        std::chrono::duration<int64, std::ratio<1, 20>>(1)
    );
}

std::string getDimensionName(Dimension& dimension) { return dimension.mName; }
std::string getDimensionName(BlockSource& region) { return getDimensionName(region.getDimension()); }
std::string getDimensionName(DimensionType const& dimId)
{
    return getDimensionName(*ll::service::getLevel()->getOrCreateDimension(dimId).lock());
}
DimensionType getDimensionId(std::string const& dimName) { return VanillaDimensions::fromString(dimName); }
NetworkIdentifier& getNetworkIdentifier(NetworkPeer& peer)
{
    static ll::DenseMap<NetworkPeer*, optional_ref<NetworkIdentifier>> mMap;
    if (auto it = mMap.find(&peer); it != mMap.end() && it->first && it->second.has_value()) { return it->second; }
    auto& connections =
        ll::service::getNetworkSystem()->mUnk61fe2a.as<std::vector<std::unique_ptr<NetworkConnection>>>();
    auto result = std::find_if(
        connections.begin(),
        connections.end(),
        [&peer](std::unique_ptr<NetworkConnection>& connection) -> bool {
            return connection->mPeer.get() == &peer;
        }
    );
    if (result != connections.end()) { return mMap[&peer] = result->get()->mId.get(); }
    std::unreachable();
}

} // namespace ila

LL_REGISTER_MOD(ila::iListenAttentively, ila::iListenAttentively::getInstance());