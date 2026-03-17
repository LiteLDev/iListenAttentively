#include "ila/event/explosion/ExplosionDamageEntityEvent.h"
#include "ExplosionProcessEntityEvent.h"
#include "ila/base/Gloabl.i.h"
#include <ll/api/event/EventRefObjSerializer.h>
#include <mc/nbt/CompoundTag.h>

namespace ila::explosion {

void ExplosionDamageEntityEvent::serialize(CompoundTag& nbt) const {
    ExplosionProcessEntityEvent::serialize(nbt);
    nbt["source"]    = serializeRefObj(mSource);
    nbt["damage"]    = mDamage;
    nbt["knockback"] = mKnockback;
    nbt["ignite"]    = mIgnite;
}

void ExplosionDamageEntityEvent::deserialize(CompoundTag const& nbt) {
    ExplosionProcessEntityEvent::deserialize(nbt);
    mDamage    = nbt["damage"];
    mKnockback = nbt["knock"];
    mIgnite    = nbt["ignite"];
}

} // namespace ila::explosion