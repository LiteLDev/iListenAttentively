#pragma once
#include "ila/event/explosion/ExplosionProcessBlockEvent.h"
#include <ila/base/Macro.h>
#include <ll/api/event/Cancellable.h>
#include <mc/deps/vanilla_components/IConstBlockSource.h>
#include <mc/deps/nbt/CompoundTag.h>
#include <mc/world/actor/ActorDamageSource.h>
#include <mc/world/item/ItemStack.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/Explosion.h>
#include <vector>

namespace ila::explosion {

class ExplosionLootBlockEvent : public ExplosionProcessBlockEvent {
private:
    std::vector<ItemStack>& mLoot;

public:
    constexpr explicit ExplosionLootBlockEvent(
        Explosion&              explosion,
        BlockPos const&         pos,
        Block const&            block,
        bool                    isExtraBlock,
        std::vector<ItemStack>& loot
    )
    : ExplosionProcessBlockEvent(explosion, pos, block, isExtraBlock),
      mLoot(loot) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

public:
    std::vector<ItemStack>& loot() { return mLoot; }
};

/** @warning This event is not available on the client side. */
class ExplosionLootingBlockEvent final : public ll::event::Cancellable<ExplosionLootBlockEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
class ExplosionLootedBlockEvent final : public ExplosionLootBlockEvent {
public:
    using ExplosionLootBlockEvent::ExplosionLootBlockEvent;
};

} // namespace ila::explosion