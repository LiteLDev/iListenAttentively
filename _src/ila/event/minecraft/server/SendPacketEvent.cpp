#include "ila/event/minecraft/server/SendPacketEvent.h"
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
#include <mc/common/SubClientId.h>
#include <mc/deps/core/utility/optional_ref.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/StringTag.h>
#include <mc/network/MinecraftPacketIds.h>
#include <mc/network/NetworkIdentifier.h>
#include <mc/network/NetworkSystem.h>
#include <mc/network/Packet.h>
#include <mc/network/ServerNetworkHandler.h>
#include <memory>
#include <string>

namespace ila::mc::inline server
{

void ISendPacketBeforeEvent::serialize(CompoundTag& nbt) const
{
    nbt["networkSystem"]     = serializeRefObj(mNetworkSystem);
    nbt["packet"]            = serializeRefObj(mPacket);
    nbt["networkIdentifier"] = serializeRefObj(mNetworkIdentifier);
    nbt["senderSubId"]       = magic_enum::enum_name(mSenderSubId);
}
void ISendPacketBeforeEvent::deserialize(CompoundTag const& nbt)
{
    mSenderSubId =
        magic_enum::enum_cast<SubClientId>(nbt["senderSubId"].get<StringTag>()).value_or(mSenderSubId);
}
optional_ref<ServerPlayer> ISendPacketBeforeEvent::player() const
{
    return ll::service::getServerNetworkHandler()->_getServerPlayer(mNetworkIdentifier, mSenderSubId);
}

void ISendPacketAfterEvent::serialize(CompoundTag& nbt) const
{
    nbt["networkSystem"]     = serializeRefObj(mNetworkSystem);
    nbt["packet"]            = serializeRefObj(mPacket);
    nbt["networkIdentifier"] = serializeRefObj(mNetworkIdentifier);
    nbt["senderSubId"]       = magic_enum::enum_name(mSenderSubId);
}
optional_ref<ServerPlayer> ISendPacketAfterEvent::player() const
{
    return ll::service::getServerNetworkHandler()->_getServerPlayer(mNetworkIdentifier, mSenderSubId);
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