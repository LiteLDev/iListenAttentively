#pragma once
#include "ila/base/Macro.h"
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/Event.h>
#include <mc/certificates/identity/PlayerAuthenticationInfo.h>
#include <mc/common/SubClientId.h>
#include <mc/deps/nbt/CompoundTag.h>
#include <mc/network/NetworkIdentifier.h>

namespace ila::server {

class ClientLoginEvent : public ll::event::Event {
private:
    NetworkIdentifier const&  mNetworkIdentifier;
    PlayerAuthenticationInfo& mAuthInfo;

public:
    constexpr explicit ClientLoginEvent(NetworkIdentifier const& networkIdentifier, PlayerAuthenticationInfo& authInfo)
    : Event(),
      mNetworkIdentifier(networkIdentifier),
      mAuthInfo(authInfo) {}

public:
    ILAPI void serialize(CompoundTag& nbt) const override;
    ILAPI void deserialize(CompoundTag const& nbt) override;

public:
    NetworkIdentifier const&  networkIdentifier() const { return mNetworkIdentifier; }
    PlayerAuthenticationInfo& authInfo() const { return mAuthInfo; }
};

/** @warning This event is not available on the client side. */
class ClientLoginingEvent final : public ll::event::Cancellable<ClientLoginEvent> {
public:
    using Cancellable::Cancellable;
};

/** @warning This event is not available on the client side. */
class ClientLoginedEvent final : public ClientLoginEvent {
public:
    using ClientLoginEvent::ClientLoginEvent;
};

} // namespace ila::server