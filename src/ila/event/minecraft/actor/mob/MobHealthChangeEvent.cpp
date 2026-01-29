#include "ila/event/minecraft/actor/mob/MobHealthChangeEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/entity/ActorEvent.h>
#include <ll/api/memory/Hook.h>
#include <magic_enum.hpp>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/StringTag.h>
#include <mc/world/attribute/AttributeBuff.h>
#include <mc/world/attribute/AttributeBuffType.h>
#include <mc/world/attribute/HealthAttributeDelegate.h>

namespace ila::mc::inline actor::inline mob
{

void MobHealthChangeBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["oldValue"] = mOlaValue;
    nbt["newValue"] = mNewValue;
    nbt["buff"]     = {
        { "amount", mBuff.mAmount },
        { "type", magic_enum::enum_name(mBuff.mType) },
        { "source", serializePtrObj(mBuff.mSource.get()) },
        { "valueAmplifier", serializePtrObj(mBuff.mValueAmplifier.get()) },
        { "durationAmplifier", serializePtrObj(mBuff.mDurationAmplifier.get()) },
        { "scale", mBuff.mScale },
        { "amplification", mBuff.mAmplification },
        { "id", mBuff.mId },
        { "operand", mBuff.mOperand },
    };
}
void MobHealthChangeBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mOlaValue    = nbt["oldValue"];
    mNewValue    = nbt["newValue"];
    mBuff.mAmount = nbt["buff"]["amount"];
    mBuff.mType =
        magic_enum::enum_cast<AttributeBuffType>(nbt["buff"]["type"].get<StringTag>()).value_or(mBuff.mType);
    mBuff.mScale         = nbt["buff"]["scale"];
    mBuff.mAmplification = nbt["buff"]["amplification"];
    mBuff.mId            = nbt["buff"]["id"];
    mBuff.mOperand       = nbt["buff"]["operand"];
}

void MobHealthChangeAfterEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["oldValue"] = mOldValue;
    nbt["newValue"] = mNewValue;
    nbt["buff"]     = {
        { "amount", mBuff.mAmount },
        { "type", magic_enum::enum_name(mBuff.mType) },
        { "source", serializePtrObj(mBuff.mSource.get()) },
        { "valueAmplifier", serializePtrObj(mBuff.mValueAmplifier.get()) },
        { "durationAmplifier", serializePtrObj(mBuff.mDurationAmplifier.get()) },
        { "scale", mBuff.mScale },
        { "amplification", mBuff.mAmplification },
        { "id", mBuff.mId },
        { "operand", mBuff.mOperand },
    };
}

LL_TYPE_INSTANCE_HOOK(
    MobHealthChangeHook,
    HookPriority::Normal,
    HealthAttributeDelegate,
    &HealthAttributeDelegate::$change,
    float,
    float                oldValue,
    float                newValue,
    AttributeBuff const& buff
)
{
    auto before = MobHealthChangeBeforeEvent(*mMob, oldValue, newValue, const_cast<AttributeBuff&>(buff));
    LLEventBus.publish(before);
    if (before.isCancelled()) { return oldValue; }
    auto result = origin(oldValue, newValue, buff);
    LLEventBus.publish(MobHealthChangeAfterEvent(*mMob, oldValue, result, const_cast<AttributeBuff&>(buff)));
    return result;
}

Event_Hook_Factory(MobHealthChange, <MobHealthChangeHook>);

} // namespace ila::mc::inline actor::inline mob
