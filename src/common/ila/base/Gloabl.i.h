#pragma once
#include "ila/utils/MemoryUtils.i.h"
#include <atomic>
#include <ll/api/event/Emitter.h>
#include <ll/api/event/EmitterBase.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/io/Logger.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/memory/Memory.h>
#include <ll/api/mod/NativeMod.h>
#include <ll/api/reflection/Deserialization.h>
#include <ll/api/reflection/Serialization.h>
#include <mc/nbt/CompoundTagVariant.h>
#include <memory>

namespace ila::inline base {

using ll::event::serializePtrObj;
using ll::event::serializeRefObj;
using ll::memory::dAccess;
using namespace ila::memory_utils;
namespace reflection {
using namespace ll::reflection;
}

ll::event::EventBus& getLLEventBus();
ll::mod::NativeMod&  getSelfMod();
ll::io::Logger&      getLogger();

} // namespace ila::inline base

namespace ll::memory {
template <class T>
constexpr FuncPtr resolveIdentifier(ila::memory_utils::internal::Address const& address) {
    return address.as<void*>();
}
} // namespace ll::memory

#define EventHookFactory(eventName, ...)                                                                               \
    static std::unique_ptr<ll::event::EmitterBase> eventName##EmitterFactory();                                        \
    class eventName##Emitter : public ll::event::Emitter<eventName##EmitterFactory, eventName> {                       \
        ll::memory::HookRegistrar __VA_ARGS__ hook;                                                                    \
    };                                                                                                                 \
    static std::unique_ptr<ll::event::EmitterBase> eventName##EmitterFactory() {                                       \
        return std::make_unique<eventName##Emitter>();                                                                 \
    }

#define EventHook(before, after, ...)                                                                                  \
    EventHookFactory(before, __VA_ARGS__);                                                                             \
    EventHookFactory(after, __VA_ARGS__);

#define HookAliasDef(hookAlias)                                                                                        \
    struct hookAlias {                                                                                                 \
        static std::atomic_uint _AutoHookCount;                                                                        \
                                                                                                                       \
        static int  hook(bool suspendThreads = true);                                                                  \
        static bool unhook(bool suspendThreads = true);                                                                \
    };

#define HookAliasImpl(hookName, hookAlias)                                                                             \
    std::atomic_uint hookAlias::_AutoHookCount{};                                                                      \
    int              hookAlias::hook(bool suspendThreads) { return hookName::hook(suspendThreads); }                   \
    bool             hookAlias::unhook(bool suspendThreads) { return hookName::unhook(suspendThreads); }