#include "ila/event/minecraft/server/RegisterCmdEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/server/commands/CommandRegistry.h>

namespace ila::mc::inline server {

void RegisterCmdBeforeEvent::serialize(CompoundTag& nbt) const {
    Event::serialize(nbt);
    nbt["name"]        = commandName();
    nbt["description"] = description();
    nbt["requirement"] = magic_enum::enum_name(requirement());
    nbt["flag1"]       = magic_enum::enum_name(flag1().value);
    nbt["flag2"]       = magic_enum::enum_name(flag2().value);
}
void RegisterCmdBeforeEvent::deserialize(CompoundTag const& nbt) {
    Event::deserialize(nbt);
    description() = nbt["description"];
    requirement() =
        magic_enum::enum_cast<CommandPermissionLevel>(nbt["requirement"].get<StringTag>()).value_or(requirement());
    flag1().value = magic_enum::enum_cast<CommandFlagValue>(nbt["flag1"].get<StringTag>()).value_or(flag1().value);
    flag2().value = magic_enum::enum_cast<CommandFlagValue>(nbt["flag2"].get<StringTag>()).value_or(flag2().value);
}
CommandRegistry&        RegisterCmdBeforeEvent::registry() const { return mRegistry; }
std::string const&      RegisterCmdBeforeEvent::commandName() const { return mName; }
std::string&            RegisterCmdBeforeEvent::description() const { return mDescription; }
CommandPermissionLevel& RegisterCmdBeforeEvent::requirement() const { return mRequirement; }
CommandFlag&            RegisterCmdBeforeEvent::flag1() const { return mFlag1; }
CommandFlag&            RegisterCmdBeforeEvent::flag2() const { return mFlag2; }

void RegisterCmdAfterEvent::serialize(CompoundTag& nbt) const {
    Event::serialize(nbt);
    nbt["name"]        = commandName();
    nbt["description"] = description();
    nbt["requirement"] = magic_enum::enum_name(requirement());
    nbt["flag1"]       = magic_enum::enum_name(flag1().value);
    nbt["flag2"]       = magic_enum::enum_name(flag2().value);
}
CommandRegistry&              RegisterCmdAfterEvent::registry() const { return mRegistry; }
std::string const&            RegisterCmdAfterEvent::commandName() const { return mName; }
std::string const&            RegisterCmdAfterEvent::description() const { return mDescription; }
CommandPermissionLevel const& RegisterCmdAfterEvent::requirement() const { return mRequirement; }
CommandFlag const&            RegisterCmdAfterEvent::flag1() const { return mFlag1; }
CommandFlag const&            RegisterCmdAfterEvent::flag2() const { return mFlag2; }

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
) {
    auto description = std::string{pDescription};
    LLEventBus.publish(RegisterCmdBeforeEvent(*this, pName, description, pRequirement, pFlag1, pFlag2));
    origin(pName, description.c_str(), pRequirement, pFlag1, pFlag2);
    LLEventBus.publish(RegisterCmdAfterEvent(*this, pName, description, pRequirement, pFlag1, pFlag2));
}

Event_Hook_Factory(RegisterCmd, <RegisterCmdEventHook>);

} // namespace ila::mc::inline server
