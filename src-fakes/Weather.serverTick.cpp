#include <mc/deps/core/math/IRandom.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/deps/shared_types/legacy/LevelEvent.h>
#include <mc/gameplayhandlers/CoordinatorResult.h>
#include <mc/world/events/EventRef.h>
#include <mc/world/events/LevelEventCoordinator.h>
#include <mc/world/events/LevelWeatherChangedEvent.h>
#include <mc/world/events/MutableLevelGameplayEvent.h>
#include <mc/world/level/ILevel.h>
#include <mc/world/level/Weather.h>
#include <mc/world/level/dimension/Dimension.h>
#include <mc/world/level/storage/GameRuleId.h>
#include <mc/world/level/storage/GameRules.h>
#include <mc/world/level/storage/LevelData.h>

#include <cmath>

void Weather::serverTick() {
    GameRuleId weatherRule{};
    weatherRule.mValue = static_cast<int>(GameRules::GameRulesIndex::DoWeatherCycle);
    if (!mDimension.mLevel.getGameRules().getBool(weatherRule, false)) {
        return;
    }

    LevelData& levelData     = mDimension.mLevel.getLevelData();
    int        lightningTime = levelData.mLightningTime - 1;
    int        rainTime      = levelData.mRainTime - 1;

    if (lightningTime > 0 && rainTime > 0) {
        levelData.mLightningTime = lightningTime;
        levelData.mRainTime      = rainTime;
        return;
    }

    bool isLightning     = mTargetLightningLevel > 0.0f;
    bool isRaining       = mTargetRainLevel > 0.0f;
    bool willBeLightning = (lightningTime <= 0) ^ isLightning;
    bool willBeRaining   = (rainTime <= 0) ^ isRaining;

    int nextLightningTime = lightningTime;
    if (nextLightningTime <= 0) {
        if (willBeLightning) {
            nextLightningTime = mRandom.nextInt(12000) + 3600;
        } else {
            nextLightningTime = mRandom.nextInt(168000) + 12000;
        }
    }

    int nextRainTime = rainTime;
    if (nextRainTime <= 0) {
        if (willBeRaining) {
            nextRainTime = mRandom.nextInt(12000) + 12000;
        } else {
            nextRainTime = mRandom.nextInt(168000) + 12000;
        }
    }

    LevelWeatherChangedEvent weatherChangedEvent{
        isRaining,
        isLightning,
        willBeRaining,
        willBeLightning,
        nextRainTime,
        nextLightningTime,
    };

    auto& eventCoordinator = mDimension.mLevel.getLevelEventCoordinator();
    if (eventCoordinator.sendEvent(EventRef<MutableLevelGameplayEvent<CoordinatorResult>>(weatherChangedEvent))
        == CoordinatorResult::Continue) {

        levelData.mLightningTime = weatherChangedEvent.mLightningTime;
        if (weatherChangedEvent.mIsLightning != weatherChangedEvent.mWillBeLightning) {
            float lightningLevel      = weatherChangedEvent.mWillBeLightning ? 1.0f : 0.0f;
            levelData.mLightningLevel = lightningLevel;
            mTargetLightningLevel     = lightningLevel;
        }

        levelData.mRainTime = weatherChangedEvent.mRainTime;
        if (weatherChangedEvent.mIsRaining != weatherChangedEvent.mWillBeRaining) {
            float rainLevel      = weatherChangedEvent.mWillBeRaining ? 1.0f : 0.0f;
            levelData.mRainLevel = rainLevel;
            mTargetRainLevel     = rainLevel;

            auto levelEvent = weatherChangedEvent.mWillBeRaining ? SharedTypes::Legacy::LevelEvent::StartRaining
                                                                 : SharedTypes::Legacy::LevelEvent::StopRaining;
            mDimension.mLevel.broadcastLevelEvent(
                levelEvent,
                Vec3::ZERO(),
                static_cast<int>(std::floor(rainLevel * 65535.0f)),
                nullptr
            );
        }

        if (weatherChangedEvent.mIsLightning != weatherChangedEvent.mWillBeLightning
            || weatherChangedEvent.mIsRaining != weatherChangedEvent.mWillBeRaining) {
            eventCoordinator.sendLevelWeatherChanged(
                mDimension.mName,
                weatherChangedEvent.mIsRaining,
                weatherChangedEvent.mIsLightning,
                weatherChangedEvent.mWillBeRaining,
                weatherChangedEvent.mWillBeLightning
            );
        }
        return;
    }

    if (lightningTime <= 0) {
        if (isLightning) {
            lightningTime = mRandom.nextInt(12000) + 3600;
        } else {
            lightningTime = mRandom.nextInt(168000) + 12000;
        }
    }

    if (rainTime <= 0) {
        if (isRaining) {
            rainTime = mRandom.nextInt(12000) + 12000;
        } else {
            rainTime = mRandom.nextInt(168000) + 12000;
        }
    }

    levelData.mLightningTime = lightningTime;
    levelData.mRainTime      = rainTime;
}
