#include "ila/base/Gloabl.hpp"

inline struct EventTest {
    EventTest();
    ~EventTest() = default;
} test;

EventTest::EventTest() {
    static std::vector<ll::event::EventId> mEventList;
    static auto const& callback = ll::event::Listener<ll::event::Event>::create([](ll::event::Event& event) -> void {
        auto const& it = std::find(mEventList.begin(), mEventList.end(), event.getId());
        if (it != mEventList.end()) {
            CompoundTag nbt;
            event.serialize(nbt);
            ila::getLogger().warn(
                "Event {0} triggered, remaining {1} events to be triggered, data: {2}",
                event.getId().name,
                mEventList.size(),
                nbt.toSnbt()
            );
            // mEventList.erase(it);
        }
    });
    for (auto const& eventName : ila::getLLEventBus().events(ll::mod::NativeMod::current()->getName())) {
        mEventList.emplace_back(eventName);
        if (!ila::getLLEventBus().addListener(callback, eventName)) {
            ila::getLogger().error("Failed to add listener for event: {0}", eventName.name);
        }
    }
}