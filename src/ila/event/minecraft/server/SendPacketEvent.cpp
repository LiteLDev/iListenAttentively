#include "ila/event/minecraft/server/SendPacketEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/service/Bedrock.h>
#include <ll/api/utils/StringUtils.h>
#include <mc/network/NetworkSystem.h>
#include <mc/network/ServerNetworkHandler.h>
#include <mc/client/renderer/rendergraph/Packet.h>

namespace ila::mc::inline server
{

void ISendPacketBeforeEvent::serialize(CompoundTag& nbt) const
{
    nbt["networkSystem"]     = serializeRefObj(networkSystem());
    nbt["packet"]            = serializeRefObj(packet());
    nbt["networkIdentifier"] = serializeRefObj(networkIdentifier());
    nbt["senderSubId"]       = magic_enum::enum_name(senderSubId());
}
void ISendPacketBeforeEvent::deserialize(CompoundTag const& nbt)
{
    senderSubId() =
        magic_enum::enum_cast<SubClientId>(nbt["senderSubId"].get<StringTag>()).value_or(senderSubId());
}
NetworkSystem&             ISendPacketBeforeEvent::networkSystem() const { return mNetworkSystem; }
Packet&                    ISendPacketBeforeEvent::packet() const { return mPacket; }
NetworkIdentifier const&   ISendPacketBeforeEvent::networkIdentifier() const { return mNetworkIdentifier; }
SubClientId&               ISendPacketBeforeEvent::senderSubId() const { return mSenderSubId; }
optional_ref<ServerPlayer> ISendPacketBeforeEvent::player() const
{
    return ll::service::getServerNetworkHandler()->_getServerPlayer(networkIdentifier(), senderSubId());
}

void ISendPacketAfterEvent::serialize(CompoundTag& nbt) const
{
    nbt["networkSystem"]     = serializeRefObj(networkSystem());
    nbt["packet"]            = serializeRefObj(packet());
    nbt["networkIdentifier"] = serializeRefObj(networkIdentifier());
    nbt["senderSubId"]       = magic_enum::enum_name(senderSubId());
}
NetworkSystem&             ISendPacketAfterEvent::networkSystem() const { return mNetworkSystem; }
Packet const&              ISendPacketAfterEvent::packet() const { return mPacket; }
NetworkIdentifier const&   ISendPacketAfterEvent::networkIdentifier() const { return mNetworkIdentifier; }
SubClientId const&         ISendPacketAfterEvent::senderSubId() const { return mSenderSubId; }
optional_ref<ServerPlayer> ISendPacketAfterEvent::player() const
{
    return ll::service::getServerNetworkHandler()->_getServerPlayer(networkIdentifier(), senderSubId());
}

LL_TYPE_INSTANCE_HOOK(
    SendPacketEventHook,
    HookPriority::Normal,
    NetworkSystem,
    &NetworkSystem::send,
    void,
    NetworkIdentifier const& id,
    Packet const&            packet,
    SubClientId              senderSubId
)
{
    auto& eventBus    = LLEventBus;
    auto  beforeEvent = SendPacketBeforeEvent<Packet>(*this, const_cast<Packet&>(packet), id, senderSubId);
    eventBus.publish(beforeEvent);
    eventBus.publish(beforeEvent, [&]() -> ll::event::EventId {
        auto packetName = std::string { magic_enum::enum_name(packet.getId()) };
        if (!packetName.ends_with("Packet")) packetName += "Packet";
        return ll::event::EventId { fmt::format(
            "{0}<class {1}>",
            ll::reflection::type_name_v<SendPacketBeforeEvent<Packet>>,
            packetName
        ) };
    }());
    if (beforeEvent.isCancelled()) return;
    origin(id, packet, senderSubId);
    auto afterEvent = SendPacketAfterEvent(*this, const_cast<Packet&>(packet), id, senderSubId);
    eventBus.publish(afterEvent);
    eventBus.publish(afterEvent, [&]() -> ll::event::EventId {
        auto packetName = std::string { magic_enum::enum_name(packet.getId()) };
        if (!packetName.ends_with("Packet")) packetName += "Packet";
        return ll::event::EventId { fmt::format(
            "{0}<class {1}>",
            ll::reflection::type_name_v<SendPacketAfterEvent<Packet>>,
            packetName
        ) };
    }());
}

static std::unique_ptr<ll::event::EmitterBase> SendPacketEventEmitterFactory();
class SendPacketEventEventEmitter : public ll::event::Emitter<SendPacketEventEmitterFactory>
{
private:
    static inline bool reg = []() -> bool {
        constexpr static auto addEvent = [](std::string const& eventName) -> void {
            LLEventBus.setEventEmitter(
                SendPacketEventEmitterFactory,
                ll::event::EventId { fmt::format(
                    "{0}<class {1}>",
                    ll::reflection::type_name_v<SendPacketBeforeEvent<Packet>>,
                    eventName
                ) }
            );
            LLEventBus.setEventEmitter(
                SendPacketEventEmitterFactory,
                ll::event::EventId { fmt::format(
                    "{0}<class {1}>",
                    ll::reflection::type_name_v<SendPacketAfterEvent<Packet>>,
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
    SendPacketEventEventEmitter() { ll::memory::HookRegistrar<SendPacketEventHook>().hook(); }
    ~SendPacketEventEventEmitter() { ll::memory::HookRegistrar<SendPacketEventHook>().unhook(); }
};
static std::unique_ptr<ll::event::EmitterBase> SendPacketEventEmitterFactory()
{
    return std::make_unique<SendPacketEventEventEmitter>();
}

} // namespace ila::mc::inline server