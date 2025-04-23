#include "ila/event/leviAntiCheat/SusClientEvent.h"
#include "ila/base/Gloabl.h"
#include <mc/platform/UUID.h>

namespace lac::punish
{
void SusClientEvent::serialize(CompoundTag& nbt) const
{
    Event::serialize(nbt);
    nbt["uuid"] = uuid().asString();
    nbt["name"] = name();
    nbt["ip"]   = ip();
}

mce::UUID const&        SusClientEvent::uuid() const { return mUuid; }
std::string_view const& SusClientEvent::name() const { return mName; }
std::string_view const& SusClientEvent::ip() const { return mIp; }
} // namespace lac::punish