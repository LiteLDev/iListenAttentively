#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Event.h>
#include <mc/server/commands/CommandPermissionLevel.h>

// clang-format off
class CommandRegistry;
class CommandFlag;
// clang-format on

namespace ila::mc::inline server
{

class RegisterCmdBeforeEvent final : public ll::event::Event
{
public:
    CommandRegistry&        mRegistry;
    std::string const&      mName;
    std::string&            mDescription;
    CommandPermissionLevel& mRequirement;
    CommandFlag&            mFlag1;
    CommandFlag&            mFlag2;

public:
    constexpr explicit RegisterCmdBeforeEvent(
        CommandRegistry&        registry,
        std::string const&      name,
        std::string&            description,
        CommandPermissionLevel& requirement,
        CommandFlag&            flag1,
        CommandFlag&            flag2
    )
        : Event()
        , mRegistry(registry)
        , mName(name)
        , mDescription(description)
        , mRequirement(requirement)
        , mFlag1(flag1)
        , mFlag2(flag2)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

};

class RegisterCmdAfterEvent final : public ll::event::Event
{
public:
    CommandRegistry&              mRegistry;
    std::string const&            mName;
    std::string const&            mDescription;
    CommandPermissionLevel const& mRequirement;
    CommandFlag const&            mFlag1;
    CommandFlag const&            mFlag2;

public:
    constexpr explicit RegisterCmdAfterEvent(
        CommandRegistry&              registry,
        std::string const&            name,
        std::string const&            description,
        CommandPermissionLevel const& requirement,
        CommandFlag const&            flag1,
        CommandFlag const&            flag2
    )
        : Event()
        , mRegistry(registry)
        , mName(name)
        , mDescription(description)
        , mRequirement(requirement)
        , mFlag1(flag1)
        , mFlag2(flag2)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;

};

} // namespace ila::mc::inline server