#pragma once
#include <ll/api/event/Emitter.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/mod/NativeMod.h>
#include <ll/api/reflection/Deserialization.h>
#include <ll/api/reflection/Serialization.h>

namespace ila::inline base {

using namespace ll::memory_literals;
using ll::event::serializePtrObj;
using ll::event::serializeRefObj;
using ll::memory::dAccess;

ll::event::EventBus& getLLEventBus();
ll::mod::NativeMod&  getSelfMod();
ll::io::Logger&      getLogger();

} // namespace ila::inline base

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