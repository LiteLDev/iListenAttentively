#include "ila/event/minecraft/server/SendMultiplePacketEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/service/Bedrock.h>
#include <ll/api/utils/StringUtils.h>
#include <mc/network/NetworkIdentifierWithSubId.h>
#include <mc/network/NetworkSystem.h>
#include <mc/network/ServerNetworkHandler.h>
#include <mc/network/packet/Packet.h>

namespace ila::mc::inline server
{

void ISendMultiplePacketBeforeEvent::serialize(CompoundTag& nbt) const
{
    nbt["networkSystem"]      = serializeRefObj(networkSystem());
    nbt["packet"]             = serializeRefObj(packet());
    nbt["networkIdentifiers"] = ListTag {};
    for (auto& networkIdentifier : networkIdentifiers())
    {
        nbt["networkIdentifiers"].push_back({ { "id", ll::event::serializeRefObj(networkIdentifier.id) },
                                              { "subId",
                                                magic_enum::enum_name(networkIdentifier.subClientId) } });
    }
}
NetworkSystem& ISendMultiplePacketBeforeEvent::networkSystem() const { return mNetworkSystem; }
Packet&        ISendMultiplePacketBeforeEvent::packet() const { return mPacket; }
std::vector<NetworkIdentifierWithSubId> const& ISendMultiplePacketBeforeEvent::networkIdentifiers() const
{
    return mNetworkIdentifiers;
}
optional_ref<ServerPlayer> ISendMultiplePacketBeforeEvent::player(
    NetworkIdentifierWithSubId const& networkIdentifier
) const
{
    return ll::service::getServerNetworkHandler()->_getServerPlayer(
        networkIdentifier.id,
        networkIdentifier.subClientId
    );
}

void ISendMultiplePacketAfterEvent::serialize(CompoundTag& nbt) const
{
    nbt["networkSystem"]      = serializeRefObj(networkSystem());
    nbt["packet"]             = serializeRefObj(packet());
    nbt["networkIdentifiers"] = ListTag {};
    for (auto& networkIdentifier : networkIdentifiers())
    {
        nbt["networkIdentifiers"].push_back({ { "id", ll::event::serializeRefObj(networkIdentifier.id) },
                                              { "subId",
                                                magic_enum::enum_name(networkIdentifier.subClientId) } });
    }
}
NetworkSystem& ISendMultiplePacketAfterEvent::networkSystem() const { return mNetworkSystem; }
Packet const&  ISendMultiplePacketAfterEvent::packet() const { return mPacket; }
std::vector<NetworkIdentifierWithSubId> const& ISendMultiplePacketAfterEvent::networkIdentifiers() const
{
    return mNetworkIdentifiers;
}
optional_ref<ServerPlayer> ISendMultiplePacketAfterEvent::player(
    NetworkIdentifierWithSubId const& networkIdentifier
) const
{
    return ll::service::getServerNetworkHandler()->_getServerPlayer(
        networkIdentifier.id,
        networkIdentifier.subClientId
    );
}

LL_TYPE_INSTANCE_HOOK(
    SendMultiplePacketEventHook,
    HookPriority::Normal,
    NetworkSystem,
    &NetworkSystem::sendToMultiple,
    void,
    std::vector<NetworkIdentifierWithSubId> const& ids,
    Packet const&                                  packet
)
{
    auto& eventBus    = LLEventBus;
    auto  beforeEvent = SendMultiplePacketBeforeEvent<Packet>(*this, const_cast<Packet&>(packet), ids);
    eventBus.publish(beforeEvent);
    eventBus.publish(beforeEvent, [&]() -> ll::event::EventIdView {
        auto packetName = std::string { magic_enum::enum_name(packet.getId()) };
        if (!packetName.ends_with("Packet")) packetName += "Packet";
        return ll::event::EventIdView { fmt::format(
            "{0}<class {1}>",
            ll::reflection::type_name_v<SendMultiplePacketBeforeEvent<Packet>>,
            packetName
        ) };
    }());
    if (beforeEvent.isCancelled()) return;
    origin(ids, packet);
    auto afterEvent = SendMultiplePacketAfterEvent(*this, const_cast<Packet&>(packet), ids);
    eventBus.publish(afterEvent);
    eventBus.publish(afterEvent, [&]() -> ll::event::EventIdView {
        auto packetName = std::string { magic_enum::enum_name(packet.getId()) };
        if (!packetName.ends_with("Packet")) packetName += "Packet";
        return ll::event::EventIdView { fmt::format(
            "{0}<class {1}>",
            ll::reflection::type_name_v<SendMultiplePacketAfterEvent<Packet>>,
            packetName
        ) };
    }());
}

static std::unique_ptr<ll::event::EmitterBase> SendMultiplePacketEventEmitterFactory();
class SendMultiplePacketEventEventEmitter : public ll::event::Emitter<SendMultiplePacketEventEmitterFactory>
{
private:
    static inline bool reg = []() -> bool {
        constexpr static auto addEvent = [](std::string const& eventName) -> void {
            LLEventBus.setEventEmitter(
                SendMultiplePacketEventEmitterFactory,
                ll::event::EventIdView { fmt::format(
                    "{0}<class {1}>",
                    ll::reflection::type_name_v<SendMultiplePacketBeforeEvent<Packet>>,
                    eventName
                ) }
            );
            LLEventBus.setEventEmitter(
                SendMultiplePacketEventEmitterFactory,
                ll::event::EventIdView { fmt::format(
                    "{0}<class {1}>",
                    ll::reflection::type_name_v<SendMultiplePacketAfterEvent<Packet>>,
                    eventName
                ) }
            );
        };
        for (auto& [id, name] : magic_enum::enum_entries<MinecraftPacketIds>())
        {
            if (id == MinecraftPacketIds::EndId) continue;
            auto packetName = std::string { name };
            if (!packetName.ends_with("Packet")) packetName += "Packet";
            addEvent(packetName);
        }
        addEvent("Packet");
        return true;
    }();

public:
    SendMultiplePacketEventEventEmitter() { ll::memory::HookRegistrar<SendMultiplePacketEventHook>().hook(); }
    ~SendMultiplePacketEventEventEmitter()
    {
        ll::memory::HookRegistrar<SendMultiplePacketEventHook>().unhook();
    }
};
static std::unique_ptr<ll::event::EmitterBase> SendMultiplePacketEventEmitterFactory()
{
    return std::make_unique<SendMultiplePacketEventEventEmitter>();
}

} // namespace ila::mc::inline server