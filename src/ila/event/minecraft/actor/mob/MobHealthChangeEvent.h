#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/entity/MobEvent.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/actor/Mob.h>

// clang-format off
class AttributeBuff;
// clang-format on

namespace ila::mc::inline actor::inline mob
{
class MobHealthChangeBeforeEvent final : public ll::event::Cancellable<ll::event::entity::MobEvent>
{
protected:
    float&         mOlaValue;
    float&         mNewValue;
    AttributeBuff& mBuff;

public:
    constexpr explicit MobHealthChangeBeforeEvent(
        Mob&           actor,
        float&         olaValue,
        float&         newValue,
        AttributeBuff& buff
    )
        : Cancellable(actor)
        , mOlaValue(olaValue)
        , mNewValue(newValue)
        , mBuff(buff)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

    ILNDAPI float&         oldValue() const;
    ILNDAPI float&         newValue() const;
    ILNDAPI AttributeBuff& buff() const;
};

class MobHealthChangeAfterEvent final : public ll::event::entity::MobEvent
{
protected:
    float const&         mOldValue;
    float const&         mNewValue;
    AttributeBuff const& mBuff;

public:
    constexpr explicit MobHealthChangeAfterEvent(
        Mob&                 actor,
        float const&         oldValue,
        float const&         newValue,
        AttributeBuff const& buff
    )
        : MobEvent(actor)
        , mOldValue(oldValue)
        , mNewValue(newValue)
        , mBuff(buff)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

    ILNDAPI float const&         oldValue() const;
    ILNDAPI float const&         newValue() const;
    ILNDAPI AttributeBuff const& buff() const;
};
} // namespace ila::mc::inline actor::inline mob