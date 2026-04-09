#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <utility>
#include <vector>

#include "ila/base/Gloabl.i.h"
#include <cstddef>
#include <cstdlib>
#include <ila/utils/MemoryUtils.i.h>
#include <ll/api/memory/Hook.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/deps/core/utility/optional_ref.h>
#include <mc/deps/ecs/EntityId.h>
#include <mc/deps/ecs/Optional.h>
#include <mc/deps/ecs/ViewT.h>
#include <mc/deps/ecs/strict/Include.h>
#include <mc/deps/ecs/strict/StrictEntityContext.h>
#include <mc/deps/vanilla_components/AABBShapeComponent.h>
#include <mc/deps/vanilla_components/CollidableMobNearFlagComponent.h>
#include <mc/deps/vanilla_components/FallingBlockFlagComponent.h>
#include <mc/deps/vanilla_components/IConstBlockSource.h>
#include <mc/deps/vanilla_components/MoveRequestComponent.h>
#include <mc/deps/vanilla_components/utilities/CollisionShapes.h>
#include <mc/entity/components/CollidableMobFlagComponent.h>
#include <mc/entity/components/LocalSpatialEntityFetcher.h>
#include <mc/entity/components/MaxAutoStepComponent.h>
#include <mc/entity/systems/move_collision_system/MoveCollisionSystem.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/block/GetCollisionShapeInterface.h>
#include <mc/world/phys/AABB.h>

class Block;

namespace BlockSourceVisitor {

struct CollisionShape {
    AABB         mAABB;
    Block const* mMaybeBlock;
    BlockPos     mBlockPos;
    bool         mIsUnloadedChunk;
};

static_assert(sizeof(CollisionShape) == 0x30);

} // namespace BlockSourceVisitor

namespace MoveCollisionSystem {

static std::vector<AABB> getFetchBoxSubtractionLocal(AABB const& newBox, AABB const& oldBox) {
    std::vector<AABB> result;

    float const localOffsetX = newBox.min.x;
    float const localOffsetY = newBox.min.y;
    float const localOffsetZ = newBox.min.z;
    float const sizeX        = newBox.max.x - newBox.min.x;
    float const sizeY        = newBox.max.y - newBox.min.y;
    float const sizeZ        = newBox.max.z - newBox.min.z;

    std::array<std::array<float, 2>, 3> oldBoxAxes{};
    oldBoxAxes[0] = {oldBox.min.x - localOffsetX, oldBox.max.x - localOffsetX};
    oldBoxAxes[1] = {oldBox.min.y - localOffsetY, oldBox.max.y - localOffsetY};
    oldBoxAxes[2] = {oldBox.min.z - localOffsetZ, oldBox.max.z - localOffsetZ};

    std::array<std::array<float, 27>, 3> pointsToEvaluate{};
    pointsToEvaluate[0][0] = oldBox.min.x - localOffsetX;
    pointsToEvaluate[0][1] = oldBox.max.x - localOffsetX;
    pointsToEvaluate[0][2] = 0.0f;
    pointsToEvaluate[1][2] = 0.0f;
    pointsToEvaluate[2][0] = oldBox.min.z - localOffsetZ;
    pointsToEvaluate[2][1] = oldBox.max.z - localOffsetZ;
    pointsToEvaluate[2][2] = 0.0f;

    std::array<std::array<float, 27>, 3> resultMins{};
    std::array<std::array<float, 27>, 3> resultMaxs{};

    int             pointCount      = 1;
    int             currentPoint    = 0;
    int             distinctBoxes   = 0;
    int             processedPoints = 0;
    float*          axisSeed        = &pointsToEvaluate[0][2];
    constexpr float epsInsert       = 0.0000099999997f;
    constexpr float epsEqual        = 0.00000011920929f;
    constexpr float epsMerge        = 0.001f;

    while (true) {
        bool pointIsBoundary = true;
        int  axisIndex       = 0;
        int  axisBase        = 0;
        int  pointBase       = currentPoint;

        while (axisIndex < 3) {
            float const pointValue = pointsToEvaluate[axisIndex][pointBase + 2];
            auto const  begin      = oldBoxAxes[axisIndex].begin();
            auto const  end        = oldBoxAxes[axisIndex].end();
            auto const  lower      = std::lower_bound(begin, end, pointValue);
            int const   minBucket  = static_cast<int>(lower - begin);

            float      axisMax   = axisIndex == 0 ? sizeX : (axisIndex == 1 ? sizeY : sizeZ);
            auto const upper     = std::lower_bound(begin, end, axisMax);
            int const  maxBucket = static_cast<int>(upper - begin);

            float maxValue = axisMax;
            if (minBucket != maxBucket) {
                maxValue = oldBoxAxes[axisIndex][minBucket];

                bool shouldInsertSplit = true;
                for (int existingPoint = 0; existingPoint < pointCount; ++existingPoint) {
                    bool same = true;
                    for (int otherAxis = 0; otherAxis < 3; ++otherAxis) {
                        float const candidate = (otherAxis == axisIndex) ? oldBoxAxes[axisIndex][minBucket] + epsInsert
                                                                         : pointsToEvaluate[otherAxis][pointBase + 2];
                        if (std::abs(candidate - pointsToEvaluate[otherAxis][existingPoint + 2]) > epsEqual) {
                            same = false;
                            break;
                        }
                    }
                    if (same) {
                        shouldInsertSplit = false;
                        break;
                    }
                }

                if (shouldInsertSplit) {
                    if (axisBase != 0) {
                        pointsToEvaluate[0][pointCount + 2] = pointsToEvaluate[0][pointBase + 2];
                    }
                    if (axisIndex != 1) {
                        pointsToEvaluate[1][pointCount + 2] = pointsToEvaluate[1][pointBase + 2];
                    }
                    if (axisIndex != 2) {
                        pointsToEvaluate[2][pointCount + 2] = pointsToEvaluate[2][pointBase + 2];
                    }
                    if (pointCount < 27) {
                        pointsToEvaluate[axisBase][pointCount + 2 + axisBase] =
                            oldBoxAxes[axisIndex][minBucket] + epsInsert;
                        ++pointCount;
                    } else {
                        return {};
                    }
                }
            }

            if (minBucket >= 0 && minBucket != 1) {
                pointIsBoundary = false;
            }

            resultMins[axisIndex][pointBase + 2] = pointValue;
            resultMaxs[axisIndex][pointBase + 2] = maxValue;
            ++axisIndex;
            ++axisBase;
            pointBase += 27;
        }

        int nextProcessed = processedPoints + 1;
        if (pointIsBoundary) {
            nextProcessed = processedPoints;
        }
        processedPoints = nextProcessed;

        ++distinctBoxes;
        ++currentPoint;
        ++axisSeed;
        if (currentPoint < pointCount) {
            continue;
        }

        if (nextProcessed <= 0) {
            return result;
        }

        for (int i = 0; i < nextProcessed; ++i) {
            float const maxX = resultMaxs[0][i + 2];
            float const maxY = resultMaxs[1][i + 2];
            float const maxZ = resultMaxs[2][i + 2];
            float const minX = resultMins[0][i + 2];
            float const minY = resultMins[1][i + 2];
            float const minZ = resultMins[2][i + 2];

            float const boxMinX = std::min(minX, maxX);
            float const boxMinY = std::min(minY, maxY);
            float const boxMinZ = std::min(minZ, maxZ);
            float const boxMaxX = std::max(minX, maxX);
            float const boxMaxY = std::max(minY, maxY);
            float const boxMaxZ = std::max(minZ, maxZ);

            if (boxMinX >= boxMaxX || boxMinY >= boxMaxY || boxMinZ >= boxMaxZ) {
                continue;
            }

            bool merged = false;
            for (AABB& existing : result) {
                float const boundMinX = std::min(existing.min.x - localOffsetX, boxMinX);
                float const boundMinY = std::min(existing.min.y - localOffsetY, boxMinY);
                float const boundMinZ = std::min(existing.min.z - localOffsetZ, boxMinZ);
                float const boundMaxX = std::max(existing.max.x - localOffsetX, boxMaxX);
                float const boundMaxY = std::max(existing.max.y - localOffsetY, boxMaxY);
                float const boundMaxZ = std::max(existing.max.z - localOffsetZ, boxMaxZ);

                float const overlapX = std::max(
                    0.0f,
                    std::min(existing.max.x - localOffsetX, boxMaxX) - std::max(existing.min.x - localOffsetX, boxMinX)
                );
                float const overlapY = std::max(
                    0.0f,
                    std::min(existing.max.y - localOffsetY, boxMaxY) - std::max(existing.min.y - localOffsetY, boxMinY)
                );
                float const overlapZ = std::max(
                    0.0f,
                    std::min(existing.max.z - localOffsetZ, boxMaxZ) - std::max(existing.min.z - localOffsetZ, boxMinZ)
                );

                float const existingVolume = (existing.max.x - existing.min.x) * (existing.max.y - existing.min.y)
                                           * (existing.max.z - existing.min.z);
                float const boxVolume     = (boxMaxX - boxMinX) * (boxMaxY - boxMinY) * (boxMaxZ - boxMinZ);
                float const boundVolume   = (boundMaxX - boundMinX) * (boundMaxY - boundMinY) * (boundMaxZ - boundMinZ);
                float const overlapVolume = overlapX * overlapY * overlapZ;

                if (std::abs((boundVolume + overlapVolume) - (existingVolume + boxVolume)) <= epsMerge) {
                    existing.min = Vec3{boundMinX + localOffsetX, boundMinY + localOffsetY, boundMinZ + localOffsetZ};
                    existing.max = Vec3{boundMaxX + localOffsetX, boundMaxY + localOffsetY, boundMaxZ + localOffsetZ};
                    merged       = true;
                    break;
                }
            }

            if (!merged) {
                result.emplace_back(
                    Vec3{boxMinX + localOffsetX, boxMinY + localOffsetY, boxMinZ + localOffsetZ},
                    Vec3{boxMaxX + localOffsetX, boxMaxY + localOffsetY, boxMaxZ + localOffsetZ}
                );
            }
        }
        return result;
    }
}

static void addCollisionShapesLocal(
    std::vector<BlockSourceVisitor::CollisionShape> const& tempCollisionShapes,
    AABB const&                                            terrainIntersectTestBox,
    MoveRequestComponent&                                  request
) {
    auto& collisionShapes = request.mCollisionShapes.get();
    auto& shapeVec        = collisionShapes.mShapes.get();
    auto& blockVec        = collisionShapes.mBlocks.get();

    std::size_t const tempCount = tempCollisionShapes.size();
    if (tempCount > shapeVec.capacity()) {
        shapeVec.reserve(tempCount);
    }
    if (tempCount > blockVec.capacity()) {
        blockVec.reserve(tempCount);
    }

    shapeVec.clear();
    blockVec.clear();
    collisionShapes.mNearUnloadedChunk = false;

    for (BlockSourceVisitor::CollisionShape const& shape : tempCollisionShapes) {
        shapeVec.emplace_back(shape.mAABB);

        CollisionShapes::BlockAndBlockPos blockAndPos{};
        blockAndPos.mBlock    = shape.mMaybeBlock;
        blockAndPos.mBlockPos = shape.mMaybeBlock ? shape.mBlockPos : BlockPos::ZERO();

        blockVec.emplace_back(blockAndPos);

        if (shape.mIsUnloadedChunk) {
            collisionShapes.mNearUnloadedChunk = true;
        }
    }

    request.mMoveCollisionLastFetchedBox.get() = terrainIntersectTestBox;
}

} // namespace MoveCollisionSystem

namespace ila::fakes {

namespace {

struct TestForCollidableMobsCallback {
    ViewT<StrictEntityContext, Include<CollidableMobFlagComponent>, AABBShapeComponent const> const* view;
    ViewT<StrictEntityContext, Include<FallingBlockFlagComponent>> const*                            fallingBlockView;
    AABB const*                                                                                      actorAABB;
    CollisionShapes*                                                                                 collisionShapes;
};

using TestForCollidableMobsImpl = void (*)(
    LocalSpatialEntityFetcher*,
    AABB const*,
    ViewT<StrictEntityContext, Include<CollidableMobFlagComponent>, AABBShapeComponent const> const*,
    TestForCollidableMobsCallback*,
    StrictEntityContext const*
);

static bool hasCollidableMobNear(Optional<CollidableMobNearFlagComponent const> const& collidableMobNear) {
    using Storage =
        entt::basic_storage<CollidableMobNearFlagComponent, EntityId, std::allocator<CollidableMobNearFlagComponent>, void>;

    if (collidableMobNear.mEnTTStorage == nullptr) {
        return false;
    }

    auto* storage = reinterpret_cast<Storage*>(const_cast<void*>(reinterpret_cast<void const*>(collidableMobNear.mEnTTStorage)));
    return storage->contains(collidableMobNear.mEntity);
}

static void addCollidableMobCollisionShapes(
    AABB const&                entityIntersectTestBox,
    AABBShapeComponent const&  aabb,
    LocalSpatialEntityFetcher& fetcher,
    ViewT<StrictEntityContext, Include<CollidableMobFlagComponent>, AABBShapeComponent const> const& collidableMobs,
    ViewT<StrictEntityContext, Include<FallingBlockFlagComponent>> const&                            fallingBlocks,
    StrictEntityContext const&                                                                       entity,
    MoveRequestComponent&                                                                            request
) {
    auto localFallingBlocks  = fallingBlocks;
    auto localCollidableMobs = collidableMobs;

    AABB const entityQueryBox{
        entityIntersectTestBox.min - Vec3{0.25f, 0.25f, 0.25f},
        entityIntersectTestBox.max + Vec3{0.25f, 0.25f, 0.25f},
    };

    auto const helper = reinterpret_cast<TestForCollidableMobsImpl>(static_cast<void*>(0x10EF530_rva));
    TestForCollidableMobsCallback callback{
        &localCollidableMobs,
        &localFallingBlocks,
        &aabb.mAABB.get(),
        &request.mCollisionShapes.get(),
    };
    helper(&fetcher, &entityQueryBox, &localCollidableMobs, &callback, &entity);
}

} // namespace

LL_STATIC_HOOK(
    MoveCollisionFetchCollisionShapesHook,
    HookPriority::Normal,
    &MoveCollisionSystem::fetchCollisionShapes,
    void,
    StrictEntityContext const&                                                                       entity,
    AABBShapeComponent const&                                                                        aabb,
    MaxAutoStepComponent const&                                                                      autoStep,
    Optional<CollidableMobNearFlagComponent const>                                                   collidableMobNear,
    MoveRequestComponent&                                                                            request,
    ViewT<StrictEntityContext, Include<CollidableMobFlagComponent>, AABBShapeComponent const> const& collidableMobs,
    ViewT<StrictEntityContext, Include<FallingBlockFlagComponent>> const&                            fallingBlocks,
    IConstBlockSource const&                                                                         blockSource,
    LocalSpatialEntityFetcher&                                                                       fetcher,
    GetCollisionShapeInterface const&                                                                collisionShape,
    std::vector<BlockSourceVisitor::CollisionShape>& tempCollisionShapes,
    std::vector<BlockSourceVisitor::CollisionShape>& scratchCollisionShapes,
    std::vector<AABB>&                               tempShapes
) {
    Vec3&       speedRef       = request.mSpeed.get();
    float const speedLengthSqr = speedRef.x * speedRef.x + speedRef.y * speedRef.y + speedRef.z * speedRef.z;
    if (speedLengthSqr > 256.0f) {
        float const invLength = 1.0f / std::sqrt(speedLengthSqr);
        speedRef.x            = invLength * speedRef.x * 16.0f;
        speedRef.y            = invLength * speedRef.y * 16.0f;
        speedRef.z            = invLength * speedRef.z * 16.0f;
    }

    Vec3 const speed     = request.mSpeed.get();
    AABB const actorAABB = aabb.mAABB.get();

    AABB movedBox = actorAABB;
    if (speed.x < 0.0f) {
        movedBox.min.x += speed.x;
    } else if (speed.x > 0.0f) {
        movedBox.max.x += speed.x;
    }
    if (speed.y < 0.0f) {
        movedBox.min.y += speed.y;
    } else if (speed.y > 0.0f) {
        movedBox.max.y += speed.y;
    }
    if (speed.z < 0.0f) {
        movedBox.min.z += speed.z;
    } else if (speed.z > 0.0f) {
        movedBox.max.z += speed.z;
    }

    float interiorMinX = actorAABB.min.x + 0.025f;
    float interiorMaxX = actorAABB.max.x - 0.025f;
    float interiorMinY = actorAABB.min.y;
    float interiorMaxY = actorAABB.max.y;
    float interiorMinZ = actorAABB.min.z + 0.025f;
    float interiorMaxZ = actorAABB.max.z - 0.025f;

    if (interiorMinX > interiorMaxX) {
        interiorMinX = (actorAABB.min.x + actorAABB.max.x) * 0.5f;
        interiorMaxX = interiorMinX;
    }
    if (interiorMinY > interiorMaxY) {
        interiorMinY = (actorAABB.min.y + actorAABB.max.y) * 0.5f;
        interiorMaxY = interiorMinY;
    }
    if (interiorMinZ > interiorMaxZ) {
        interiorMinZ = (actorAABB.min.z + actorAABB.max.z) * 0.5f;
        interiorMaxZ = interiorMinZ;
    }

    float const autoStepValue = autoStep.mValue;

    AABB terrainIntersectTestBox{
        Vec3{
             std::min(std::min(interiorMinX + speed.x, interiorMinX), movedBox.min.x),
             std::min(interiorMinY + autoStepValue * -1.01f,                                                 movedBox.min.y),
             std::min(std::min(interiorMinZ + speed.z,       interiorMinZ),   movedBox.min.z),
             },
        Vec3{
             std::max(std::max(interiorMaxX + speed.x,                                        interiorMaxX),                                                                     movedBox.max.x),
             std::max(interiorMaxY + autoStepValue * -1.01f, movedBox.max.y),
             std::max(std::max(interiorMaxZ + speed.z, interiorMaxZ), movedBox.max.z),
             },
    };

    AABB autoStepBox = actorAABB;
    if (speed.x < 0.0f) {
        autoStepBox.min.x += speed.x;
    } else if (speed.x > 0.0f) {
        autoStepBox.max.x += speed.x;
    }
    if (speed.y + autoStepValue < 0.0f) {
        autoStepBox.min.y += speed.y + autoStepValue;
    } else if (speed.y + autoStepValue > 0.0f) {
        autoStepBox.max.y += speed.y + autoStepValue;
    }
    if (speed.z < 0.0f) {
        autoStepBox.min.z += speed.z;
    } else if (speed.z > 0.0f) {
        autoStepBox.max.z += speed.z;
    }

    AABB stepColumnBox = actorAABB;
    if (autoStepValue < 0.0f) {
        stepColumnBox.min.y += autoStepValue;
    } else if (autoStepValue > 0.0f) {
        stepColumnBox.max.y += autoStepValue;
    }

    AABB entityIntersectTestBox{
        Vec3{
             std::min({actorAABB.min.x, movedBox.min.x, stepColumnBox.min.x, terrainIntersectTestBox.min.x}),
             std::min(
                {actorAABB.min.y - (std::abs(speed.y) + 0.2f),
                 autoStepBox.min.y,
                 stepColumnBox.min.y,
                 terrainIntersectTestBox.min.y}
            ), std::min({actorAABB.min.z, movedBox.min.z, stepColumnBox.min.z, terrainIntersectTestBox.min.z}),
             },
        Vec3{
             std::max({actorAABB.max.x, movedBox.max.x, stepColumnBox.max.x, terrainIntersectTestBox.max.x}),
             std::max({actorAABB.max.y + 0.08f, autoStepBox.max.y, stepColumnBox.max.y, terrainIntersectTestBox.max.y}),
             std::max({actorAABB.max.z, movedBox.max.z, stepColumnBox.max.z, terrainIntersectTestBox.max.z}),
             },
    };

    AABB const& lastFetchedBoxRef = request.mMoveCollisionLastFetchedBox.get();
    if (entityIntersectTestBox.min.x >= lastFetchedBoxRef.min.x
        && entityIntersectTestBox.max.x <= lastFetchedBoxRef.max.x
        && entityIntersectTestBox.min.y >= lastFetchedBoxRef.min.y
        && entityIntersectTestBox.max.y <= lastFetchedBoxRef.max.y
        && entityIntersectTestBox.min.z >= lastFetchedBoxRef.min.z
        && entityIntersectTestBox.max.z <= lastFetchedBoxRef.max.z) {
        return;
    }

    tempCollisionShapes.clear();

    AABB const& lastFetchedBox = request.mMoveCollisionLastFetchedBox.get();
    bool const  canReuseLastFetch =
        lastFetchedBox.max.x > lastFetchedBox.min.x && lastFetchedBox.max.y > lastFetchedBox.min.y
        && lastFetchedBox.max.z > lastFetchedBox.min.z && lastFetchedBox.max.x > terrainIntersectTestBox.min.x
        && lastFetchedBox.min.x < terrainIntersectTestBox.max.x && terrainIntersectTestBox.min.y < lastFetchedBox.max.y
        && terrainIntersectTestBox.max.y > lastFetchedBox.min.y && terrainIntersectTestBox.min.z < lastFetchedBox.max.z
        && terrainIntersectTestBox.max.z > lastFetchedBox.min.z;

    if (!canReuseLastFetch) {
        blockSource.fetchCollisionShapesAndBlocks(
            tempCollisionShapes,
            terrainIntersectTestBox,
            true,
            optional_ref{collisionShape},
            &tempShapes
        );
    } else {
        std::vector<AABB> const fetchBoxSubtraction = MoveCollisionSystem::getFetchBoxSubtractionLocal(
            terrainIntersectTestBox,
            request.mMoveCollisionLastFetchedBox.get()
        );

        if (!fetchBoxSubtraction.empty()) {
            scratchCollisionShapes.clear();
            for (AABB const& subBox : fetchBoxSubtraction) {
                blockSource.fetchCollisionShapesAndBlocks(
                    scratchCollisionShapes,
                    subBox,
                    true,
                    optional_ref{collisionShape},
                    &tempShapes
                );
                tempCollisionShapes
                    .insert(tempCollisionShapes.end(), scratchCollisionShapes.begin(), scratchCollisionShapes.end());
            }
        } else {
            return;
        }
    }

    MoveCollisionSystem::addCollisionShapesLocal(tempCollisionShapes, terrainIntersectTestBox, request);

    if (hasCollidableMobNear(collidableMobNear)) {
        addCollidableMobCollisionShapes(
            entityIntersectTestBox,
            aabb,
            fetcher,
            collidableMobs,
            fallingBlocks,
            entity,
            request
        );
    }
}

} // namespace ila::fakes
