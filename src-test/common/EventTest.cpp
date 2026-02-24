#include "ila/base/Gloabl.i.h"
#include "ila/event/minecraft/block/BlockPistonEvent.h"
#include "mc/nbt/CompoundTag.h"
#include <mc/world/level/Level.h>
#include <mc/world/level/chunk/LevelChunk.h>
#include <mc/world/level/chunk/LevelChunkEventManager.h>
#include <ll/api/event/client/ClientJoinLevelEvent.h>
#include <mc/deps/core/utility/pub_sub/SubscriptionContext.h>
#include <ll/api/utils/StacktraceUtils.h>
#include <mc/deps/core/utility/pub_sub/ConnectPosition.h>

inline struct EventTest {
    EventTest();
    ~EventTest() = default;
} test;

EventTest::EventTest() {
    ila::getLLEventBus().emplaceListener<ila::mc::BlockPistonExtendEvent>([](ila::mc::BlockPistonExtendEvent& event) {
        CompoundTag tag;
        event.serialize(tag);
        std::cout << tag.toSnbt(SnbtFormat::Console | SnbtFormat::Colored, 0) << std::endl;
        event.cancel();
    });
    ila::getLLEventBus().emplaceListener<ila::mc::BlockPistonRetractEvent>([](ila::mc::BlockPistonRetractEvent& event) {
        CompoundTag tag;
        event.serialize(tag);
        std::cout << tag.toSnbt(SnbtFormat::Console | SnbtFormat::Colored, 0) << std::endl;
    });
}