#include "ila/base/Gloabl.i.h"
#include "ila/event/minecraft/packet/SendPacketEvent.h"

inline struct EventTest {
    EventTest();
    ~EventTest() = default;
} test;

EventTest::EventTest() {
    ila::getLLEventBus().emplaceListener<ila::mc::SendingPacketEvent>([](ila::mc::SendingPacketEvent& event) {
        std::cout << "SendingPacketEvent(" << (event.networkSystem().isServer() ? "Server" : "Client") << ") -> "
                  << event.packet().getName() << std::endl;
    });
    ila::getLLEventBus().emplaceListener<ila::mc::SentPacketEvent>([](ila::mc::SentPacketEvent& event) {
        std::cout << "SentPacketEvent(" << (event.networkSystem().isServer() ? "Server" : "Client") << ") -> "
                  << event.packet().getName() << std::endl;
    });
}