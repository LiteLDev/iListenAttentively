#include "ila/event/minecraft/server/SendMultiplePacketEvent.h"
#include "ila/base/Gloabl.h"
#include <fmt/format.h>
#include <ll/api/event/Emitter.h>
#include <ll/api/event/EmitterBase.h>
#include <ll/api/event/EventId.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/reflection/TypeName.h>
#include <ll/api/service/Bedrock.h>
#include <ll/api/utils/StringUtils.h>
#include <magic_enum.hpp>
#include <mc/deps/core/utility/optional_ref.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/network/MinecraftPacketIds.h>
#include <mc/network/NetworkIdentifierWithSubId.h>
#include <mc/network/NetworkSystem.h>
#include <mc/network/Packet.h>
#include <mc/network/ServerNetworkHandler.h>
#include <memory>
#include <string>
#include <vector>

namespace ila::mc::inline server
{

void ISendMultiplePacketBeforeEvent::serialize(CompoundTag& nbt) const
{
    nbt["networkSystem"]      = serializeRefObj(mNetworkSystem);
    nbt["packet"]             = serializeRefObj(mPacket);
    nbt["networkIdentifiers"] = ListTag {};
    for (auto& networkIdentifier : mNetworkIdentifiers)
    {
        nbt["networkIdentifiers"].push_back(
            { { "id", ll::event::serializeRefObj(networkIdentifier.id) },
              { "subId", magic_enum::enum_name(networkIdentifier.subClientId) } }
        );
    }
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
    nbt["networkSystem"]      = serializeRefObj(mNetworkSystem);
    nbt["packet"]             = serializeRefObj(mPacket);
    nbt["networkIdentifiers"] = ListTag {};
    for (auto& networkIdentifier : mNetworkIdentifiers)
    {
        nbt["networkIdentifiers"].push_back(
            { { "id", ll::event::serializeRefObj(networkIdentifier.id) },
              { "subId", magic_enum::enum_name(networkIdentifier.subClientId) } }
        );
    }
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
    eventBus.publish(beforeEvent, [&]() -> ll::event::EventId {
        auto packetName = std::string { magic_enum::enum_name(packet.getId()) };
        if (!packetName.ends_with("Packet")) packetName += "Packet";
        return ll::event::EventId { fmt::format(
            "{0}<class {1}>",
            ll::reflection::type_name_v<SendMultiplePacketBeforeEvent<Packet>>,
            packetName
        ) };
    }());
    if (beforeEvent.isCancelled()) return;
    origin(ids, packet);
    auto afterEvent = SendMultiplePacketAfterEvent(*this, const_cast<Packet&>(packet), ids);
    eventBus.publish(afterEvent);
    eventBus.publish(afterEvent, [&]() -> ll::event::EventId {
        auto packetName = std::string { magic_enum::enum_name(packet.getId()) };
        if (!packetName.ends_with("Packet")) packetName += "Packet";
        return ll::event::EventId { fmt::format(
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
                ll::event::EventId { fmt::format(
                    "{0}<class {1}>",
                    ll::reflection::type_name_v<SendMultiplePacketBeforeEvent<Packet>>,
                    eventName
                ) }
            );
            LLEventBus.setEventEmitter(
                SendMultiplePacketEventEmitterFactory,
                ll::event::EventId { fmt::format(
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