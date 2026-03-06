#pragma once
#include <mc/nbt/CompoundTag.h>

struct LevelEventGenericPacketPayload {
public:
    int         mEventId;
    CompoundTag mData;
};