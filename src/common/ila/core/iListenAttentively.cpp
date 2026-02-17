#include "ila/core/iListenAttentively.hpp"
#include "ila/base/Gloabl.hpp"
#include "ila/utils/RandomColorLogFormatter.hpp"
#include <ll/api/mod/RegisterHelper.h>

namespace ila::inline mod {

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

} // namespace ila::inline mod

namespace ila::inline base {
ll::event::EventBus& getLLEventBus() { return ll::event::EventBus::getInstance(); }
ll::mod::NativeMod&  getSelfMod() { return iListenAttentively::getInstance().getSelf(); }
ll::io::Logger&      getLogger() { return getSelfMod().getLogger(); }
} // namespace ila::inline base

LL_REGISTER_MOD(ila::iListenAttentively, ila::iListenAttentively::getInstance());