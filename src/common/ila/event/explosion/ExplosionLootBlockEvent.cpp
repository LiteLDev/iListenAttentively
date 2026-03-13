#include "ila/event/explosion/ExplosionLootBlockEvent.h"
#include "ila/base/Gloabl.i.h"
#include <mc/world/item/SaveContextFactory.h>
#include <mc/world/item/ItemStack.h>
#include <mc/world/item/Item.h>

namespace ila::mc::inline explosion {

void ExplosionLootBlockEvent::serialize(CompoundTag& nbt) const {
    ExplosionProcessBlockEvent::serialize(nbt);
    nbt["loot"] = mLoot | std::views::transform([](ItemStack const& item) -> CompoundTag {
        return std::move(*item.save(*SaveContextFactory::createCloneSaveContext()));
    }) | std::ranges::to<ListTag>();
}

void ExplosionLootBlockEvent::deserialize(CompoundTag const& nbt) {
    ExplosionProcessBlockEvent::deserialize(nbt);
    // clang-format off
    mLoot.assign_range(
        nbt["loot"].get<ListTag>()
            | std::views::transform([](CompoundTagVariant const& nbt) -> ItemStack {
                auto  result = ItemStack::EMPTY_ITEM();
                result._loadItem(nbt.get<CompoundTag>());
                if (auto item = result.mItem; item) {
                    item->fixupCommon(result);
                    if (result.getAuxValue() == 0x7FFF) {
                        result.mAuxValue = 0;
                    }
                }
                return result;
            })
            | std::ranges::to<std::vector>()
    );
}

} // namespace ila::mc::inline explosion