#include "ila/iListenAttentively.h"
#include "ila/base/Gloabl.h"
#include <ll/api/chrono/GameChrono.h>
#include <ll/api/coro/CoroTask.h>
#include <ll/api/mod/RegisterHelper.h>
#include <ll/api/service/Bedrock.h>
#include <ll/api/thread/ServerThreadExecutor.h>
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
    ll::coro::keepThis([func { std::move(func) }]() -> ll::coro::CoroTask<> {
        co_await ll::chrono::ticks(1);
        func();
        co_return;
    }).launch(ll::thread::ServerThreadExecutor::getDefault());
}

std::string getDimensionName(Dimension& dimension) { return dimension.mName; }
std::string getDimensionName(BlockSource& region) { return getDimensionName(region.getDimension()); }
std::string getDimensionName(DimensionType const& dimid)
{
    return getDimensionName(*ll::service::getLevel()->getOrCreateDimension(dimid).lock());
}
DimensionType getDimensionId(std::string const& dimName) { return VanillaDimensions::fromString(dimName); }

} // namespace ila

LL_REGISTER_MOD(ila::iListenAttentively, ila::iListenAttentively::getInstance());