#include "ila/event/legacyMoney/MoneyChangeEvent.h"
#include "ila/base/Gloabl.h"
#include <windows.h>

namespace ila::legacyMoney
{

void MoneyChangeBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["type"]  = magic_enum::enum_name(getType());
    nbt["from"]  = getFromXuid();
    nbt["to"]    = getToXuid();
    nbt["value"] = getValue();
}
void MoneyChangeBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Cancellable::deserialize(nbt);
    mFromXuid = nbt["from"];
    mToXuid   = nbt["to"];
    mValue    = nbt["value"];
}
LLMoneyEventType const& MoneyChangeBeforeEvent::getType() const { return mType; }
std::string&            MoneyChangeBeforeEvent::getFromXuid() const { return mFromXuid; }
std::string&            MoneyChangeBeforeEvent::getToXuid() const { return mToXuid; }
llong&                  MoneyChangeBeforeEvent::getValue() const { return mValue; }

void MoneyChangeAfterEvent::serialize(CompoundTag& nbt) const
{
    nbt["type"]  = magic_enum::enum_name(getType());
    nbt["from"]  = getFromXuid();
    nbt["to"]    = getToXuid();
    nbt["value"] = getValue();
}
LLMoneyEventType const& MoneyChangeAfterEvent::getType() const { return mType; }
std::string const&      MoneyChangeAfterEvent::getFromXuid() const { return mFromXuid; }
std::string const&      MoneyChangeAfterEvent::getToXuid() const { return mToXuid; }
llong const&            MoneyChangeAfterEvent::getValue() const { return mValue; }

static bool isRealTrans = true;

LL_STATIC_HOOK(
    AddMoneyHook,
    HookPriority::Normal,
    GetProcAddress(GetModuleHandleW(L"LegacyMoney.dll"), "LLMoney_Add"),
    bool,
    std::string xuid,
    llong       money
)
{
    static std::string fromXuid    = "";
    auto               beforeEvent = MoneyChangeBeforeEvent(LLMoneyEventType::Add, fromXuid, xuid, money);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return false; }
    isRealTrans = false;
    auto result = origin(xuid, money);
    isRealTrans = true;
    if (result) { LLEventBus.publish(MoneyChangeAfterEvent(LLMoneyEventType::Add, fromXuid, xuid, money)); }
    return result;
}

LL_STATIC_HOOK(
    ReduceMoneyHook,
    HookPriority::Normal,
    GetProcAddress(GetModuleHandleW(L"LegacyMoney.dll"), "LLMoney_Reduce"),
    bool,
    std::string xuid,
    llong       money
)
{
    static std::string fromXuid    = "";
    auto               beforeEvent = MoneyChangeBeforeEvent(LLMoneyEventType::Reduce, fromXuid, xuid, money);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return false; }
    isRealTrans = false;
    auto result = origin(xuid, money);
    isRealTrans = true;
    if (result)
    {
        LLEventBus.publish(MoneyChangeAfterEvent(LLMoneyEventType::Reduce, fromXuid, xuid, money));
    }
    return result;
}

LL_STATIC_HOOK(
    SetMoneyHook,
    HookPriority::Normal,
    GetProcAddress(GetModuleHandleW(L"LegacyMoney.dll"), "LLMoney_Set"),
    bool,
    std::string xuid,
    llong       money
)
{
    static std::string fromXuid    = "";
    auto               beforeEvent = MoneyChangeBeforeEvent(LLMoneyEventType::Set, fromXuid, xuid, money);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return false; }
    isRealTrans = false;
    auto result = origin(xuid, money);
    isRealTrans = true;
    if (result) { LLEventBus.publish(MoneyChangeAfterEvent(LLMoneyEventType::Set, fromXuid, xuid, money)); }
    return result;
}

LL_STATIC_HOOK(
    TransMoneyHook,
    HookPriority::Normal,
    GetProcAddress(GetModuleHandleW(L"LegacyMoney.dll"), "LLMoney_Trans"),
    bool,
    std::string        fromXuid,
    std::string        toXuid,
    llong              value,
    std::string const& note
)
{
    if (!isRealTrans) return origin(fromXuid, toXuid, value, note);
    auto beforeEvent = MoneyChangeBeforeEvent(LLMoneyEventType::Trans, fromXuid, toXuid, value);
    LLEventBus.publish(beforeEvent);
    if (beforeEvent.isCancelled()) { return false; }
    auto result = origin(fromXuid, toXuid, value, note);
    if (result)
    {
        LLEventBus.publish(MoneyChangeAfterEvent(LLMoneyEventType::Trans, fromXuid, toXuid, value));
    }
    return result;
}

Event_Hook_Factory(MoneyChange, <AddMoneyHook, ReduceMoneyHook, SetMoneyHook, TransMoneyHook>)

} // namespace ila::legacyMoney