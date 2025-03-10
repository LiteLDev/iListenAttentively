#include "ila/base/Gloabl.h"

inline struct EventTest
{
    EventTest();
    ~EventTest() = default;
} test;

EventTest::EventTest()
{
    // static std::vector<ll::event::EventIdView> mEventList;
    // static auto const&                         callback =
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