#pragma once
#include <functional>
#include <ll/api/base/StdInt.h>
#include <mc/world/events/BlockEventListener.h>
#include <mc/world/events/EventCoordinatorPimpl.h>
#include <mc/world/events/EventResult.h>
#include <thread>
#include <vector>

template <>
class EventCoordinatorPimpl<BlockEventListener> {
public:
    using EventFuncPtr = std::function<EventResult(BlockEventListener&)>;

public:
    std::vector<BlockEventListener*> mListeners;
    std::vector<EventFuncPtr>        mEventsToProcess;
    std::vector<BlockEventListener*> mPendingRegistrations;
    bool                             mHasPendingRegistrations;
    std::thread::id                  mThreadId;
    bool                             mThreadIdInitialized;
    uint                             mThreadCheckIndex;

public:
    ~EventCoordinatorPimpl() = default;
};