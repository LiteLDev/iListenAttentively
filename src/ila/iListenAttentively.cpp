#include "ila/iListenAttentively.h"
#include "ila/RandomColorLogFormatter.h"
#include "ila/base/Gloabl.h"
#include <algorithm>
#include <chrono>
#include <fmt/format.h>
#include <functional>
#include <ll/api/base/StdInt.h>
#include <ll/api/data/IndirectValue.h>
#include <ll/api/io/Formatter.h>
#include <ll/api/mod/RegisterHelper.h>
#include <ll/api/service/Bedrock.h>
#include <ll/api/thread/ServerThreadExecutor.h>
#include <mc/_HeaderOutputPredefine.h>
#include <mc/network/NetworkConnection.h>
#include <mc/network/NetworkSystem.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/dimension/Dimension.h>
#include <mc/world/level/dimension/VanillaDimensions.h>
#include <ratio>
#include <string>
#include <utility>
#include <vector>

namespace ila {

iListenAttentively& iListenAttentively::getInstance() {
    static iListenAttentively instance;
    return instance;
}

bool iListenAttentively::load() {
    getSelf().getLogger().setFormatter(
        ll::makePolymorphic<RandomColorLogFormatter>(
            "{3:.3%T.} {2} {1} {0}",
            ll::io::Formatter::supportColorLog(),
            0b0010
        )
    );
    printLogo();
    return true;
}

bool iListenAttentively::enable() { return true; }

bool iListenAttentively::disable() { return true; }

void iListenAttentively::printLogo() const {
    std::vector<std::string> output = {
        R"(    ___   _          _       )",
        R"(   |_ _| | |        / \      )",
        R"(    | |  | |       / _ \     )",
        R"(    | |  | |___   / ___ \    )",
        R"(   |___| |_____| /_/   \_\   )",
        R"(                             )",
        fmt::format("iListenAttentively v{0}", getSelf().getManifest().version->to_string()),
        fmt::format("Author: {0}", "MiracleForest")
    };
    auto center = std::ranges::max_element(output, {}, &std::string::size)->size();
    for (auto& line : output) {
        getSelf().getLogger().info(fmt::format("{0:^{1}}", line, center));
    }
}

void nextTick(std::function<void()> const& func) {
    ll::thread::ServerThreadExecutor::getDefault().executeAfter(
        [func{func}]() -> void { func(); },
        std::chrono::duration<int64, std::ratio<1, 20>>(1)
    );
}

std::string getDimensionName(Dimension& dimension) { return dimension.mName; }
std::string getDimensionName(BlockSource& region) { return getDimensionName(region.getDimension()); }
std::string getDimensionName(DimensionType const& dimId) {
    return getDimensionName(*ll::service::getLevel()->getOrCreateDimension(dimId).lock());
}
DimensionType getDimensionId(std::string const& dimName) { return VanillaDimensions::fromString(dimName); }

} // namespace ila

LL_REGISTER_MOD(ila::iListenAttentively, ila::iListenAttentively::getInstance());
