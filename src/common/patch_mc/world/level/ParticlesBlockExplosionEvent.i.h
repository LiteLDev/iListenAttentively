#pragma once
#include <mc/deps/core/math/Vec3.h>
#include <mc/nbt/CompoundTagVariant.h>

class ParticlesBlockExplosionEvent {
public:
    float             mRadius;
    Vec3              mOrigin;
    std::vector<Vec3> mPositions;

public:
    MCNAPI_C void load(::CompoundTag const& data);
    std::unique_ptr<CompoundTag> save() const;
};