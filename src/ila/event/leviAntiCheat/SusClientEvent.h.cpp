#include "ila/event/leviAntiCheat/SusClientEvent.h"
#include "ila/base/Gloabl.h"
#include "ila/event/leviAntiCheat/LeviAntiCheat.hpp"

namespace ila::lac
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

Event_Listener_Factory(SusClient)
{
    mListeners.emplace_back(LLEventBus.emplaceListener<::lac::punish::SusClientEvent>(
        [](::lac::punish::SusClientEvent& event) -> void {
            auto ilaEvent = SusClientEvent { *event.mUuid, *event.mName, *event.mIp };
            LLEventBus.publish(ilaEvent);
            event.setCancelled(ilaEvent.isCancelled());
        }
    ));
}

} // namespace ila::lac