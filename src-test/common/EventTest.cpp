#include "ila/base/Gloabl.i.h"
#include <ll/api/utils/StacktraceUtils.h>
#include "ila/event/minecraft/server/ServerPongEvent.h"

inline struct EventTest {
    EventTest();
    ~EventTest() = default;
} test;

EventTest::EventTest() {
    ila::getLLEventBus().emplaceListener<ila::mc::SendingServerPongEvent>([](ila::mc::SendingServerPongEvent& event) {
        CompoundTag nbt;
        event.serialize(nbt);
        std::cout << nbt.toSnbt(SnbtFormat::Colored | SnbtFormat::Console, 0) << std::endl;
    });
    ila::getLLEventBus().emplaceListener<ila::mc::SentServerPongEvent>([](ila::mc::SentServerPongEvent& event) {
        CompoundTag nbt;
        event.serialize(nbt);
        std::cout << nbt.toSnbt(SnbtFormat::Colored | SnbtFormat::Console, 0) << std::endl;
    });
}