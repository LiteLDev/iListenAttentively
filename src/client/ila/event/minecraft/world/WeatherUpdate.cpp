#include "ila/base/Gloabl.i.h"
#include "ila/event/minecraft/world/WeatherUpdate.i.h"
#include <ll/api/service/Bedrock.h>
#include <ll/api/thread/ServerThreadExecutor.h>
#include <mc/client/game/ClientInstance.h>
#include <mc/network/GameConnectionInfo.h>
#include <mc/world/level/Weather.h>

namespace ila::mc::inline world {

LL_TYPE_INSTANCE_HOOK(
    WeatherUpdateHook2,
    HookPriority::Normal,
    Weather,
    &Weather::$levelEvent,
    void,
    SharedTypes::Legacy::LevelEvent type,
    Vec3 const&                     pos,
    int                             data
) {
    using enum SharedTypes::Legacy::LevelEvent;
    constexpr static std::array<SharedTypes::Legacy::LevelEvent, 4> weatherTypes = {
        StartRaining,
        StartThunderstorm,
        StopRaining,
        StopThunderstorm,
    };

    if (auto client = ll::service::getClientInstance()) {
        if (auto info = client->getGameConnectionInfo(); info && info->mType == Social::ConnectionType::Local) {
            return origin(type, pos, data);
        }
    }
    if (!std::ranges::any_of(weatherTypes, [type](auto t) { return type == t; })) return origin(type, pos, data);

    auto typeBeforeUpdate = getTypeFromLevels(mTargetRainLevel, mTargetLightningLevel);
    origin(type, pos, data);
    auto typeImmediatelyAfterUpdate = getTypeFromLevels(mTargetRainLevel, mTargetLightningLevel);

    ll::thread::ServerThreadExecutor::getDefault().executeAfter(
        [typeBeforeUpdate, typeImmediatelyAfterUpdate, this]() -> void {
        auto typeAfterDelay = getTypeFromLevels(mTargetRainLevel, mTargetLightningLevel);

        if (typeBeforeUpdate != typeAfterDelay && typeAfterDelay == typeImmediatelyAfterUpdate) {
            WeatherUpdatedEvent event(
                {typeBeforeUpdate, ll::chrono::ticks::zero()},
                {typeAfterDelay, ll::chrono::ticks::zero()}
            );
            ll::event::EventBus::getInstance().publish(event);
        }
    },
        ll::chrono::ticks(1)
    );
}

EventHook(WeatherUpdatingEvent, WeatherUpdatedEvent, <WeatherUpdateEventHook1, WeatherUpdateHook2>);

} // namespace ila::mc::inline world