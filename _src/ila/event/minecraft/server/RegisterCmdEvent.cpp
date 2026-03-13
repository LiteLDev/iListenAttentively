#include "ila/event/server/RegisterCmdEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/server/commands/CommandRegistry.h>

namespace ila::mc::inline server
{

void RegisterCmdBeforeEvent::serialize(CompoundTag& nbt) const
{
    Event::serialize(nbt);
    nbt["name"]        = mName;
    nbt["description"] = mDescription;
    nbt["requirement"] = magic_enum::enum_name(mRequirement);
    nbt["flag1"]       = magic_enum::enum_name(mFlag1.value);
    nbt["flag2"]       = magic_enum::enum_name(mFlag2.value);
}
void RegisterCmdBeforeEvent::deserialize(CompoundTag const& nbt)
{
    Event::deserialize(nbt);
    mDescription = nbt["description"];
    mRequirement = magic_enum::enum_cast<CommandPermissionLevel>(nbt["requirement"].get<StringTag>())
                        .value_or(mRequirement);
    mFlag1.value =
        magic_enum::enum_cast<CommandFlagValue>(nbt["flag1"].get<StringTag>()).value_or(mFlag1.value);
    mFlag2.value =
        magic_enum::enum_cast<CommandFlagValue>(nbt["flag2"].get<StringTag>()).value_or(mFlag2.value);
}

void RegisterCmdAfterEvent::serialize(CompoundTag& nbt) const
{
    Event::serialize(nbt);
    nbt["name"]        = mName;
    nbt["description"] = mDescription;
    nbt["requirement"] = magic_enum::enum_name(mRequirement);
    nbt["flag1"]       = magic_enum::enum_name(mFlag1.value);
    nbt["flag2"]       = magic_enum::enum_name(mFlag2.value);
}

LL_TYPE_INSTANCE_HOOK(
    RegisterCmdEventHook,
    HookPriority::Normal,
    CommandRegistry,
    &CommandRegistry::registerCommand,
    void,
    std::string const&     pName,
    char const*            pDescription,
    CommandPermissionLevel pRequirement,
    CommandFlag            pFlag1,
    CommandFlag            pFlag2
)
{
    auto description = std::string { pDescription };
    LLEventBus.publish(RegisterCmdBeforeEvent(*this, pName, description, pRequirement, pFlag1, pFlag2));
    origin(pName, description.c_str(), pRequirement, pFlag1, pFlag2);
    LLEventBus.publish(RegisterCmdAfterEvent(*this, pName, description, pRequirement, pFlag1, pFlag2));
}

Event_Hook_Factory(RegisterCmd, <RegisterCmdEventHook>);

} // namespace ila::mc::inline server