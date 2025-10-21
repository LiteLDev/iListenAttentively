#include "ila/event/minecraft/server/ReceivePacketEvent.h"
#include "ila/base/Gloabl.h"
#include <ll/api/service/Bedrock.h>
#include <mc/deps/core/debug/BedrockLog.h>
#include <mc/deps/core/utility/ReadOnlyBinaryStream.h>
#include <mc/deps/ecs/gamerefs_entity/EntityContext.h>
#include <mc/deps/ecs/gamerefs_entity/GameRefsEntity.h>
#include <mc/deps/game_refs/WeakRef.h>
#include <mc/entity/components/UserEntityIdentifierComponent.h>
#include <mc/gameplayhandlers/HandlerResult.h>
#include <mc/network/IPacketHandlerDispatcher.h>
#include <mc/network/MinecraftPackets.h>
#include <mc/network/NetworkConnection.h>
#include <mc/network/NetworkSystem.h>
#include <mc/network/Packet.h>
#include <mc/network/ServerNetworkHandler.h>
#include <mc/scripting/event_handlers/ScriptServerNetworkEventHandler.h>
#include <mc/world/events/IncomingPacketEvent.h>


template<>
struct GameplayHandlerResult<CoordinatorResult>
{
    HandlerResult     mHandlerResult;
    CoordinatorResult mReturnValue;
};

namespace ila::mc::inline server
{

void IReceivePacketBeforeEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["packet"]            = serializeRefObj(packet());
    nbt["networkIdentifier"] = serializeRefObj(networkIdentifier());
}
Packet&                    IReceivePacketBeforeEvent::packet() const { return mPacket; }
NetworkIdentifier const&   IReceivePacketBeforeEvent::networkIdentifier() const { return mNetworkIdentifier; }
optional_ref<ServerPlayer> IReceivePacketBeforeEvent::player() const
{
    return ll::service::getServerNetworkHandler()->_getServerPlayer(
        networkIdentifier(),
        packet().mSenderSubId
    );
}

void IReceivePacketAfterEvent::serialize(CompoundTag& nbt) const
{
    Cancellable::serialize(nbt);
    nbt["packet"]            = serializeRefObj(packet());
    nbt["networkIdentifier"] = serializeRefObj(networkIdentifier());
}
Packet const&              IReceivePacketAfterEvent::packet() const { return mPacket; }
NetworkIdentifier const&   IReceivePacketAfterEvent::networkIdentifier() const { return mNetworkIdentifier; }
optional_ref<ServerPlayer> IReceivePacketAfterEvent::player() const
{
    return ll::service::getServerNetworkHandler()->_getServerPlayer(
        networkIdentifier(),
        packet().mSenderSubId
    );
}

thread_local static NetworkConnection* mCurrentNetworkConnection = nullptr;

LL_TYPE_INSTANCE_HOOK(
    ReceivePacketEventHook1,
    HookPriority::Normal,
    NetworkSystem,
    &NetworkSystem::_sortAndPacketizeEvents,
    bool,
    NetworkConnection&                    connection,
    std::chrono::steady_clock::time_point endTime
)
{
    mCurrentNetworkConnection = &connection;
    auto result               = origin(connection, endTime);
    mCurrentNetworkConnection = nullptr;
    return result;
}

LL_TYPE_INSTANCE_HOOK(
    ReceivePacketEventHook2,
    HookPriority::Normal,
    ScriptServerNetworkEventHandler,
    &ScriptServerNetworkEventHandler::$handleEvent,
    GameplayHandlerResult<CoordinatorResult>,
    IncomingPacketEvent& packetEvent
)
{
    if (auto result = origin(packetEvent); result.mHandlerResult == HandlerResult::BypassListeners
                                           || result.mReturnValue == CoordinatorResult::Cancel)
    {
        return result;
    }
    auto                 networkSystem = ll::service::getNetworkSystem();
    ReadOnlyBinaryStream stream(networkSystem->mReceiveBuffer.get(), false);
    auto                 header = stream.getUnsignedVarInt();
    if (!header.has_value())
    {
        va_list args {};
        BedrockLog::log_va(
            BedrockLog::LogCategory::LogArea,
            { 1 },
            BedrockLog::LogRule::DefaultRules,
            LogAreaID::LogAreaNetwork,
            static_cast<uint>(Bedrock::LogLevel::Error().mType),
            __FUNCTION__,
            __LINE__,
            header.error().mError.message().c_str(),
            args
        );
    }
    auto packet = MinecraftPackets::createPacket(static_cast<MinecraftPacketIds>(header.value() & 0x3ff));
    if (!packet) { return { HandlerResult::BypassListeners, CoordinatorResult::Cancel }; }
    if (!mCurrentNetworkConnection || mCurrentNetworkConnection->mShouldCloseConnection)
    {
        return { HandlerResult::NotifyListeners, CoordinatorResult::Continue };
    }
    auto now                                   = std::chrono::steady_clock::now();
    mCurrentNetworkConnection->mLastPacketTime = now;
    packet->mReceiveTimepoint                  = now;
    if (auto result = packet->checkSize(stream.mView.size() - stream.mReadPointer, true); !result.has_value())
    {
        return { HandlerResult::BypassListeners, CoordinatorResult::Cancel };
    }
    if (auto result = packet->read(stream); !result.has_value())
    {
        return { HandlerResult::BypassListeners, CoordinatorResult::Cancel };
    }
    if (!packet->mHandler) { return { HandlerResult::BypassListeners, CoordinatorResult::Cancel }; }

    auto beforeEvent = ReceivePacketBeforeEvent<Packet>(*packet, mCurrentNetworkConnection->mId);
    LLEventBus.publish(beforeEvent);
    LLEventBus.publish(beforeEvent, [&]() -> ll::event::EventId {
        auto packetName = std::string { magic_enum::enum_name(packet->getId()) };
        if (!packetName.ends_with("Packet")) packetName += "Packet";
        return ll::event::EventId { fmt::format(
            "{0}<class {1}>",
            ll::reflection::type_name_v<ReceivePacketBeforeEvent<Packet>>,
            packetName
        ) };
    }());
    if (beforeEvent.isCancelled()) { return { HandlerResult::BypassListeners, CoordinatorResult::Cancel }; }
    packet->mHandler->handle(mCurrentNetworkConnection->mId, ll::service::getServerNetworkHandler(), packet);
    auto afterEvent = ReceivePacketAfterEvent(*packet, mCurrentNetworkConnection->mId);
    LLEventBus.publish(afterEvent);
    LLEventBus.publish(afterEvent, [&]() -> ll::event::EventId {
        auto packetName = std::string { magic_enum::enum_name(packet->getId()) };
        if (!packetName.ends_with("Packet")) packetName += "Packet";
        return ll::event::EventId { fmt::format(
            "{0}<class {1}>",
            ll::reflection::type_name_v<ReceivePacketAfterEvent<Packet>>,
            packetName
        ) };
    }());
    return { HandlerResult::BypassListeners, CoordinatorResult::Cancel };
}

static std::unique_ptr<ll::event::EmitterBase> ReceivePacketEventEmitterFactory();
class ReceivePacketEventEventEmitter : public ll::event::Emitter<ReceivePacketEventEmitterFactory>
{
private:
    static inline bool reg = []() -> bool {
        constexpr static auto addEvent = [](std::string const& eventName) -> void {
            LLEventBus.setEventEmitter(
                ReceivePacketEventEmitterFactory,
                ll::event::EventId { fmt::format(
                    "{0}<class {1}>",
                    ll::reflection::type_name_v<ReceivePacketBeforeEvent<Packet>>,
                    eventName
                ) }
            );
            LLEventBus.setEventEmitter(
                ReceivePacketEventEmitterFactory,
                ll::event::EventId { fmt::format(
                    "{0}<class {1}>",
                    ll::reflection::type_name_v<ReceivePacketAfterEvent<Packet>>,
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
    ReceivePacketEventEventEmitter()
    {
        ll::memory::HookRegistrar<ReceivePacketEventHook1, ReceivePacketEventHook2>().hook();
    }
    ~ReceivePacketEventEventEmitter()
    {
        ll::memory::HookRegistrar<ReceivePacketEventHook1, ReceivePacketEventHook2>().unhook();
    }
};
static std::unique_ptr<ll::event::EmitterBase> ReceivePacketEventEmitterFactory()
{
    return std::make_unique<ReceivePacketEventEventEmitter>();
}

} // namespace ila::mc::inline server