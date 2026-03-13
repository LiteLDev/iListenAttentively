#include "ll/api/event/Event.h"
#include "ila/base/Gloabl.i.h"
#include "ila/event/server/ClientLoginEvent.h"
#include "ll/api/event/EventId.h"
#include "ll/api/event/Listener.h"
#include <ll/api/utils/StacktraceUtils.h>

inline struct EventTest {
    EventTest();
    ~EventTest() = default;
} test;

EventTest::EventTest() {
    ila::getLLEventBus().emplaceListener<ila::mc::ClientLoginingEvent>([](ila::mc::ClientLoginingEvent& event) {
        event.authInfo().XboxLiveName = "zimuya";
    });
    ila::getLLEventBus().emplaceListener<ila::mc::ClientLoginedEvent>([](ila::mc::ClientLoginedEvent& event) {
        event.authInfo().XboxLiveName = "萱宝最可爱~";
    });
}