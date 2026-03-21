#include "ll/api/event/Event.h"
#include "ila/base/Gloabl.i.h"
#include "ila/event/block/fire/FireRemoveEvent.h"
#include "ila/event/explosion/ExplosionCollisionOffsetEvent.h"
#include "ila/event/block/fire/FireBurnBlockEvent.h"
#include "ila/event/block/fire/FireSpreadEvent.h"
#include "ll/api/event/Cancellable.h"
#include "ll/api/event/EventId.h"
#include "ll/api/event/Listener.h"
#include "ll/api/event/ListenerBase.h"
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <iostream>
#include <ll/api/event/server/ServerStartedEvent.h>
#include <ll/api/utils/StacktraceUtils.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/Tag.h>
#include <ostream>
#include <ranges>
#include <string_view>
#include <utility>
#include <vector>

inline struct EventTest {
    EventTest();
    ~EventTest() = default;
} test;

EventTest::EventTest() {
    ila::getLLEventBus().emplaceListener<ll::event::ServerStartedEvent>([](auto&) {
        auto ids = ila::getLLEventBus().events()
                 | std::views::filter([](std::pair<std::string_view, ll::event::EventIdView> const& id) {
            return id.second.name.contains("Fire") && id.second.name.contains("ila") && !id.second.name.contains("Ag");
        }) | std::ranges::to<std::vector>();

        ila::getLLEventBus().emplaceListener<ila::block::FireBurningBlockEvent>(
            [](ila::block::FireBurningBlockEvent& event) { event.cancel(); },
            ll::event::EventPriority::Low
        );
        ila::getLLEventBus().emplaceListener<ila::block::FireSpreadingEvent>(
            [](ila::block::FireSpreadingEvent& event) { event.cancel(); },
            ll::event::EventPriority::Low
        );

        std::cout << "events: "
                  << fmt::format(
                         "{}",
                         fmt::join(
                             ids | std::views::transform([](auto&& event) { return event.second.name; })
                                 | std::ranges::to<std::vector>(),
                             ", "
                         )
                     )
                  << std::endl;

        auto listener = ll::event::Listener<ll::event::Event>::create([](ll::event::Event& event) {
            CompoundTag nbt;
            event.serialize(nbt);
            std::cout << nbt.toSnbt(SnbtFormat::Colored | SnbtFormat::Console, 0) << std::endl;
        });

        for (auto& id : ids) {
            ila::getLLEventBus().addListener(listener, id.second);
        }
    });
}
