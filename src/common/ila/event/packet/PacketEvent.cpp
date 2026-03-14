#include "ila/event/packet/PacketEvent.h"
#include "ila/base/Gloabl.i.h"

namespace ila::packet {

void PacketEvent::serialize(CompoundTag& nbt) const {
    Event::serialize(nbt);
    nbt["network_system"]     = serializeRefObj(mNetworkSystem);
    nbt["network_identifier"] = serializeRefObj(mNetworkIdentifier);
    reflection::serialize_to(nbt["sender_sub_id"], mSenderSubId).value();
    nbt["packet"] = serializeRefObj(mPacket);
}

} // namespace ila::packet