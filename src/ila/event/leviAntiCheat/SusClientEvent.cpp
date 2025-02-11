#include "ila/event/leviAntiCheat/SusClientEvent.h"
#include "ila/base/Gloabl.h"

namespace lac::punish
{
void SusClientEvent::serialize(CompoundTag& nbt) const
{
    Event::serialize(nbt);
    nbt["uuid"] = getUuid().asString();
    nbt["name"] = getName();
    nbt["ip"]   = getIp();
}

mce::UUID const&        SusClientEvent::getUuid() const { return mUuid; }
std::string_view const& SusClientEvent::getName() const { return mName; }
std::string_view const& SusClientEvent::getIp() const { return mIp; }
} // namespace lac::punish