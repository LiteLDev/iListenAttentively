#include "ila/event/minecraft/world/actor/MobHealthChangeEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/world/attribute/AttributeBuff.h>
#include <mc/world/attribute/HealthAttributeDelegate.h>

namespace ila::mc::inline world::inline actor
{

void MobHealthChangeBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["oldValue"] = oldValue();
    nbt["newValue"] = newValue();
    nbt["buff"]     = {
        { "amount", buff().mAmount },
        { "type", magic_enum::enum_name(buff().mType) },
        { "source", serializePtrObj(buff().mSource.get()) },
        { "valueAmplifier", serializePtrObj(buff().mValueAmplifier.get()) },
        { "durationAmplifier", serializePtrObj(buff().mDurationAmplifier.get()) },
        { "scale", buff().mScale },
        { "amplification", buff().mAmplification },
        { "id", buff().mId },
        { "operand", buff().mOperand },
    };
}
void MobHealthChangeBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    oldValue()     = nbt["oldValue"];
    newValue()     = nbt["newValue"];
    buff().mAmount = nbt["buff"]["amount"];
    buff().mType =
        magic_enum::enum_cast<AttributeBuffType>(nbt["buff"]["type"].get<StringTag>()).value_or(buff().mType);
    buff().mScale         = nbt["buff"]["scale"];
    buff().mAmplification = nbt["buff"]["amplification"];
    buff().mId            = nbt["buff"]["id"];
    buff().mOperand       = nbt["buff"]["operand"];
}
float&         MobHealthChangeBeforeEvent::oldValue() const { return mOlaValue; }
float&         MobHealthChangeBeforeEvent::newValue() const { return mNewValue; }
AttributeBuff& MobHealthChangeBeforeEvent::buff() const { return mBuff; }

void MobHealthChangeAfterEvent::serialize(CompoundTag& nbt) const
{
    ActorEvent::serialize(nbt);
    nbt["oldValue"] = oldValue();
    nbt["newValue"] = newValue();
    nbt["buff"]     = {
        { "amount", buff().mAmount },
        { "type", magic_enum::enum_name(buff().mType) },
        { "source", serializePtrObj(buff().mSource.get()) },
        { "valueAmplifier", serializePtrObj(buff().mValueAmplifier.get()) },
        { "durationAmplifier", serializePtrObj(buff().mDurationAmplifier.get()) },
        { "scale", buff().mScale },
        { "amplification", buff().mAmplification },
        { "id", buff().mId },
        { "operand", buff().mOperand },
    };
}
float const&         MobHealthChangeAfterEvent::oldValue() const { return mOldValue; }
float const&         MobHealthChangeAfterEvent::newValue() const { return mNewValue; }
AttributeBuff const& MobHealthChangeAfterEvent::buff() const { return mBuff; }

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

} // namespace ila::mc::inline world::inline actor