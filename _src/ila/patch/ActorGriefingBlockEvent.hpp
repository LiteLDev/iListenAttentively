#pragma once
#include "mc/_HeaderOutputPredefine.h"
#include "mc/deps/core/math/Vec3.h"
#include "mc/deps/ecs/gamerefs_entity/GameRefsEntity.h"
#include "mc/deps/game_refs/WeakRef.h"

class Block;
class BlockSourceHandle;
class EntityContext;

struct ActorGriefingBlockEvent
{
public:
    WeakRef<EntityContext>             mActorContext;
    gsl::not_null<Block const*>        mBlock;
    Vec3                               mPos;
    std::shared_ptr<BlockSourceHandle> mBlockSourceHandle;

public:
    ActorGriefingBlockEvent(
        WeakRef<EntityContext>             actorContext,
        gsl::not_null<Block const*>        block,
        Vec3                               pos,
        std::shared_ptr<BlockSourceHandle> blockSourceHandle
    )
        : mActorContext(actorContext)
        , mBlock(block)
        , mPos(std::move(pos))
        , mBlockSourceHandle(blockSourceHandle)
    {
    }
};