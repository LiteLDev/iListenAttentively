#include "ila/base/Gloabl.i.h"
#include "patch_mc/world/level/ParticlesBlockExplosionEvent.i.h"
#include <functional>
#include <ila/utils/MemoryUtils.i.h>
#include <mc/nbt/CompoundTag.h>
#include <memory>

std::unique_ptr<CompoundTag> ParticlesBlockExplosionEvent::save() const {
    using namespace ila;
    // clang-format off
    return std::bind(
        "48 89 5C 24 ?? 55 56 57 41 54 "
        "41 55 41 56 41 57 48 8D AC 24 "
        "?? ?? ?? ?? 48 81 EC ?? ?? ?? "
        "?? 0F 29 B4 24 ?? ?? ?? ?? 48 "
        "8B 05 ?? ?? ?? ?? 48 33 C4 48 "
        "89 85 ?? ?? ?? ?? 4C 8B FA 48 "
        "89 55 ?? 4C 8B E9"_sig.as<
            decltype(&ParticlesBlockExplosionEvent::save)
        >(),
        this
    )();
    // clang-format on
}