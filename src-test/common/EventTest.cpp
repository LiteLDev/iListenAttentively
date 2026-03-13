#include "ll/api/event/Event.h"
#include "ila/base/Gloabl.i.h"
#include "ila/event/explosion/ExplosionCollisionOffsetEvent.h"
#include "ila/event/explosion/ExplosionDestroyBlockEvent.h"
#include "ll/api/event/Cancellable.h"
#include "ll/api/event/EventId.h"
#include "ll/api/event/Listener.h"
#include <ll/api/utils/StacktraceUtils.h>
#include <ranges>

inline struct EventTest {
    EventTest();
    ~EventTest() = default;
} test;

EventTest::EventTest() {
    auto ids =
        ila::getLLEventBus().events()
        | std::views::filter([](std::pair<std::string_view, ll::event::EventIdView> const& id) {
        return id.second.name.contains("Explo") && id.second.name.contains("Event");
    }) | std::ranges::to<std::vector>();

    auto listener = ll::event::Listener<ll::event::Cancellable<ll::event::Event>>::create([](ll::event::Cancellable<ll::event::Event>& event) {
        if (event.getId() == ll::event::getEventId<ila::mc::ExplosionDestroyBlockingEvent>) event.cancel();
        CompoundTag nbt;
        event.serialize(nbt);
        std::cout << nbt.toSnbt(SnbtFormat::Colored | SnbtFormat::Console | SnbtFormat::PrettyFilePrint, 2) << std::endl;
    });

    for (auto& id : ids) {
        ila::getLLEventBus().addListener(listener, id.second);
    }
}