#include "ila/event/minecraft/packet/PacketEvent.h"
#include "ila/base/Gloabl.i.h"

namespace ila::mc::inline packet {

void PacketEvent::serialize(CompoundTag& nbt) const {
    Event::serialize(nbt);
    nbt["network_system"]     = serializeRefObj(mNetworkSystem);
    nbt["network_identifier"] = serializeRefObj(mNetworkIdentifier);
    nbt["sender_sub_id"]      = ll::reflection::serialize<CompoundTagVariant>(mSenderSubId).value();
    nbt["packet"]             = serializeRefObj(mPacket);
}

} // namespace ila::mc::inline packet