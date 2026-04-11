#include <memory>

#include <ll/api/memory/Hook.h>
#include <mc/deps/ecs/EntityId.h>
#include <mc/deps/ecs/Optional.h>
#include <mc/deps/vanilla_components/ActorDataFlagComponent.h>
#include <mc/deps/vanilla_components/PlayerIsSleepingFlagComponent.h>
#include <mc/entity/components/IsHorizontalPoseFlagComponent.h>
#include <mc/util/VanillaOffsetSystemUtil.h>
#include <mc/world/actor/ActorFlags.h>

namespace ila::fakes {

LL_AUTO_STATIC_HOOK(
    VanillaOffsetSystemUtilGetCameraOffsetHook,
    HookPriority::Normal,
    0x140F1A3A0ULL,
    float,
    ActorDataFlagComponent const&                   actorDataFlag,
    Optional<PlayerIsSleepingFlagComponent const>   isSleepingFlag,
    Optional<IsHorizontalPoseFlagComponent const>   isHorizontalFlag,
    float                                           sneakHeight
) {
    using HorizontalStorage =
        entt::basic_storage<IsHorizontalPoseFlagComponent, EntityId, std::allocator<IsHorizontalPoseFlagComponent>, void>;
    using SleepingStorage =
        entt::basic_storage<PlayerIsSleepingFlagComponent, EntityId, std::allocator<PlayerIsSleepingFlagComponent>, void>;

    if (isHorizontalFlag.mEnTTStorage != nullptr) {
        auto* storage = reinterpret_cast<HorizontalStorage*>(
            const_cast<void*>(reinterpret_cast<void const*>(isHorizontalFlag.mEnTTStorage))
        );
        if (storage->contains(isHorizontalFlag.mEntity)) {
            return 0.40000001f;
        }
    }

    if (actorDataFlag.getStatusFlag(ActorFlags::Sneaking)) {
        return 1.62001f - sneakHeight;
    }

    if (isSleepingFlag.mEnTTStorage != nullptr) {
        auto* storage = reinterpret_cast<SleepingStorage*>(
            const_cast<void*>(reinterpret_cast<void const*>(isSleepingFlag.mEnTTStorage))
        );
        if (storage->contains(isSleepingFlag.mEntity)) {
            return 0.2f;
        }
    }

    return 1.62001f;
}

} // namespace ila::fakes
