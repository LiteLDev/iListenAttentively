#include "ll/api/event/Event.h"
#include "ila/base/Gloabl.i.h"
#include "ila/event/player/PlayerShieldBlockEvent.h"
#include "ll/api/base/Containers.h"
#include "ll/api/event/EventId.h"
#include "ll/api/event/Listener.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/item/ItemStackBase.h"
#include "mc/nbt/Tag.h"
#include <ll/api/utils/StacktraceUtils.h>

inline struct EventTest {
    EventTest();
    ~EventTest() = default;
} test;

EventTest::EventTest() {
    ila::getLLEventBus().emplaceListener<ila::player::PlayerShieldBlockingEvent>(
        [](ila::player::PlayerShieldBlockingEvent& event) {
        CompoundTag nbt;
        event.serialize(nbt);
        std::cout << nbt.toSnbt(SnbtFormat::PrettyConsolePrint, 2) << std::endl;
    }
    );
    ila::getLLEventBus().emplaceListener<ila::player::PlayerShieldBlockedEvent>(
        [](ila::player::PlayerShieldBlockedEvent& event) {
        CompoundTag nbt;
        event.serialize(nbt);
        std::cout << nbt.toSnbt(SnbtFormat::PrettyConsolePrint, 2) << std::endl;
    }
    );
}