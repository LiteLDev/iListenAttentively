#include "ila/base/Gloabl.h"
#include "ila/event/minecraft/world/RedstoneUpdateEvent.h"
#include "ila/event/minecraft/world/level/levelgen/structure/VillageFeatureEvent.h"
#include <mc/world/level/BlockPos.h>

inline struct EventTest {
    EventTest();
    ~EventTest() = default;
} test;

EventTest::EventTest() {

    LLEventBus.emplaceListener<ila::mc::world::RedstoneUpdateBeforeEvent>(
        [](ila::mc::world::RedstoneUpdateBeforeEvent& event) -> void {
            auto const& pos = event.pos();
            SelfLogger.info(
                "RedstoneUpdateBeforeEvent pos=({}, {}, {}), strength={}, isFirstTime={}, dim={}",
                pos.x,
                pos.y,
                pos.z,
                event.strength(),
                event.isFirstTime(),
                ila::getDimensionName(event.blockSource())
            );
        }
    );

    ll::event::EventBus::getInstance().emplaceListener<ila::mc::levelgen::VillageFeatureConstructionEvent>(
        []([[maybe_unused]] ila::mc::levelgen::VillageFeatureConstructionEvent& ev) {}
    );

    // static std::vector<ll::event::EventId> mEventList;
    // static auto const&                     callback =
    //     ll::event::Listener<ll::event::Event>::create([](ll::event::Event& event) -> void {
    //         auto const& it = std::find(mEventList.begin(), mEventList.end(), event.getId());
    //         if (it != mEventList.end())
    //         {
    //             CompoundTag nbt;
    //             event.serialize(nbt);
    //             SelfLogger.warn(
    //                 "Event {0} triggered, remaining {1} events to be triggered, data: {2}",
    //                 event.getId().name,
    //                 mEventList.size(),
    //                 nbt.toSnbt()
    //             );
    //             mEventList.erase(it);
    //         }
    //     });
    // for (auto const& eventName : LLEventBus.events(ll::mod::NativeMod::current()->getName()))
    // {
    //     mEventList.emplace_back(eventName);
    //     if (!LLEventBus.addListener(callback, eventName))
    //     {
    //         SelfLogger.error("Failed to add listener for event: {0}", eventName.name);
    //     }
    // }
}
