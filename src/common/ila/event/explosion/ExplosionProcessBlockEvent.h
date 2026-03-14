#pragma once
#include "ila/event/explosion/ExplosionEvent.h"

namespace ila::explosion {

class ExplosionProcessBlockEvent : public ExplosionEvent {
private:
    BlockPos const& mPos;
    Block const&    mBlock;
    bool            mIsExtraBlock;

public:
    constexpr explicit ExplosionProcessBlockEvent(
        Explosion&      explosion,
        BlockPos const& pos,
        Block const&    block,
        bool            isExtraBlock
    )
    : ExplosionEvent(explosion),
      mPos(pos),
      mBlock(block),
      mIsExtraBlock(isExtraBlock) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;

public:
    BlockPos const& pos() const { return mPos; }
    Block const&    block() const { return mBlock; }
    bool            isExtraBlock() const { return mIsExtraBlock; }
};

/** @warning This event is not available on the client side. */
class ExplosionProcessBlockingEvent final : public ll::event::Cancellable<ExplosionProcessBlockEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
class ExplosionProcessBlockedEvent final : public ExplosionProcessBlockEvent {
public:
    using ExplosionProcessBlockEvent::ExplosionProcessBlockEvent;
};

} // namespace ila::explosion