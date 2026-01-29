#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/player/ServerPlayerEvent.h>
#include <mc/deps/shared_types/legacy/ContainerType.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/server/ServerPlayer.h>
#include <mc/world/ContainerID.h>

// clang-format off
class BlockPos;
struct ActorUniqueID;
// clang-format on

namespace ila::mc::inline actor::inline player
{
class PlayerOpenContainerBeforeEvent final
    : public ll::event::Cancellable<ll::event::player::ServerPlayerEvent>
{
public:
    BlockPos&                           mPos;
    ContainerID&                        mContainerId;
    SharedTypes::Legacy::ContainerType& mContainerType;
    ActorUniqueID&                      mContainerActorId;

public:
    constexpr explicit PlayerOpenContainerBeforeEvent(
        ServerPlayer&                       player,
        BlockPos&                           pos,
        ContainerID&                        containerId,
        SharedTypes::Legacy::ContainerType& containerType,
        ActorUniqueID&                      containerActorId
    )
        : Cancellable(player)
        , mPos(pos)
        , mContainerId(containerId)
        , mContainerType(containerType)
        , mContainerActorId(containerActorId)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;
};

class PlayerOpenContainerAfterEvent final : public ll::event::player::ServerPlayerEvent
{
public:
    BlockPos const&                           mPos;
    ContainerID const&                        mContainerId;
    SharedTypes::Legacy::ContainerType const& mContainerType;
    ActorUniqueID const&                      mContainerActorId;

public:
    constexpr explicit PlayerOpenContainerAfterEvent(
        ServerPlayer&                             player,
        BlockPos const&                           pos,
        ContainerID const&                        containerId,
        SharedTypes::Legacy::ContainerType const& containerType,
        ActorUniqueID const&                      containerActorId
    )
        : ServerPlayerEvent(player)
        , mPos(pos)
        , mContainerId(containerId)
        , mContainerType(containerType)
        , mContainerActorId(containerActorId)
    {
    }

    ILAPI void serialize(CompoundTag& nbt) const override;
};
} // namespace ila::mc::inline actor::inline player