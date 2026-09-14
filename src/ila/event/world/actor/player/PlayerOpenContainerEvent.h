#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/player/PlayerEvent.h>

// clang-format off
class BlockPos;
struct ActorUniqueID;
// clang-format on

namespace ila::mc::inline world::inline actor::inline player {
class PlayerOpenContainerBeforeEvent final : public ll::event::Cancellable<ll::event::player::PlayerEvent> {
protected:
    BlockPos&                           mPos;
    SharedTypes::Legacy::ContainerType& mContainerType;
    ActorUniqueID&                      mContainerActorId;

public:
    constexpr explicit PlayerOpenContainerBeforeEvent(
        Player&                             player,
        BlockPos&                           pos,
        SharedTypes::Legacy::ContainerType& containerType,
        ActorUniqueID&                      containerActorId
    )
    : Cancellable(player),
      mPos(pos),
      mContainerType(containerType),
      mContainerActorId(containerActorId) {}

    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

    ILNDAPI BlockPos& containerBlockPos() const;
    ILNDAPI SharedTypes::Legacy::ContainerType& containerType() const;
    ILNDAPI ActorUniqueID&                      containerActorId() const;
};

class PlayerOpenContainerAfterEvent final : public ll::event::player::PlayerEvent {
protected:
    BlockPos const&                           mPos;
    SharedTypes::Legacy::ContainerType const& mContainerType;
    ActorUniqueID const&                      mContainerActorId;

public:
    constexpr explicit PlayerOpenContainerAfterEvent(
        Player&                                   player,
        BlockPos const&                           pos,
        SharedTypes::Legacy::ContainerType const& containerType,
        ActorUniqueID const&                      containerActorId
    )
    : PlayerEvent(player),
      mPos(pos),
      mContainerType(containerType),
      mContainerActorId(containerActorId) {}

    ILAPI void serialize(CompoundTag& nbt) const override;

    ILNDAPI BlockPos const& containerBlockPos() const;
    ILNDAPI SharedTypes::Legacy::ContainerType const& containerType() const;
    ILNDAPI ActorUniqueID const&                      containerActorId() const;
};
} // namespace ila::mc::inline world::inline actor::inline player
