#include "ila/event/block/BlockPistonEvent.h"
#include "ila/base/Gloabl.i.h"
#include <ll/api/base/StdInt.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/event/world/WorldEvent.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/reflection/Serialization.h>
#include <mc/common/FacingID.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/block/PistonBlock.h>
#include <mc/world/level/block/VanillaStates.h>
#include <mc/world/level/block/actor/PistonBlockActor.h>
#include <mc/world/level/block/actor/PistonState.h>
#include <mc/world/level/dimension/Dimension.h>
#include <mc/world/redstone/circuit/CircuitSystem.h>

namespace ila::block {

void BlockPistonEvent::serialize(CompoundTag& nbt) const {
    WorldEvent::serialize(nbt);
    nbt["piston"]       = serializeRefObj(mPiston);
    nbt["piston_block"] = serializeRefObj(mPistonBlock);
    reflection::serialize_to(nbt["facing"], mFacing).value();
}

enum class PistonStateEx : schar {
    // PistonState uses 0-3
    ExpandingCancelled  = 4, // iListenAttentively
    RetractingCancelled = 5, // iListenAttentively
};

LL_TYPE_INSTANCE_HOOK(
    BlockPistonEventHook,
    HookPriority::Normal,
    PistonBlockActor,
    &PistonBlockActor::$tick,
    void,
    BlockSource& region
) {
    bool extending  = false;
    bool retracting = false;

    auto& dimension = region.mDimension;

    if (dimension.mCircuitSystemTickRate >= dimension.CIRCUIT_TICK_RATE) {
        auto& circuit = dimension.mCircuitSystem;
        if (auto strength = circuit->getStrength(mPosition); strength != -1) {
            if (strength <= 0) {
                if (mState == PistonState::Expanded && mNewState == PistonState::Expanded) {
                    retracting = true;
                } else if (mState == PistonState::Retracted
                           && static_cast<PistonStateEx>(mNewState) == PistonStateEx::ExpandingCancelled) {
                    mNewState = PistonState::Retracted;
                }
            } else {
                if (mState == PistonState::Retracted && mNewState == PistonState::Retracted) {
                    extending = true;
                } else if (mState == PistonState::Expanded
                           && static_cast<PistonStateEx>(mNewState) == PistonStateEx::RetractingCancelled) {
                    mNewState = PistonState::Expanded;
                }
            }
        }
    }

    if (extending || retracting) {
        auto& block = region.getBlock(mPosition);
        auto  face  = block.getState<FacingID>(VanillaStates::FacingDirection()).value_or(FacingID::Down);

        if (extending) {
            if (!eventPromise(BlockPistonExtendEvent{region, *this, block, face}).publish()) {
                mNewState = static_cast<PistonState>(PistonStateEx::ExpandingCancelled);
            }
        } else {
            if (!eventPromise(BlockPistonRetractEvent{region, *this, block, face}).publish()) {
                mNewState = static_cast<PistonState>(PistonStateEx::RetractingCancelled);
            }
        }
    }

    origin(region);
}

EventHook(BlockPistonExtendEvent, BlockPistonRetractEvent, <BlockPistonEventHook>);

} // namespace ila::block