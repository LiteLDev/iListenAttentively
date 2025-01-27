#include "ila/base/Gloabl.h"
#include "ila/include_all.h"
#include <ll/api/event/server/ServerStartedEvent.h>
#include <ll/api/service/GamingStatus.h>

struct EventTest
{
public:
    EventTest();
    ~EventTest() = default;
};

inline struct StartTest
{
    StartTest()
    {
        if (ll::getGamingStatus() != ll::GamingStatus::Running)
        {
            LLEventBus.emplaceListener<ll::event::ServerStartedEvent>(
                [](ll::event::ServerStartedEvent&) -> void { static EventTest test; }
            );
        }
        else { static EventTest test; }
    }
} StartTest;


EventTest::EventTest() {}