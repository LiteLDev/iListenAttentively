#pragma once
#include <mc/deps/nbt/CompoundTag.h>

struct LevelEventGenericPacketPayload {
public:
    int         mEventId;
    CompoundTag mData;
};