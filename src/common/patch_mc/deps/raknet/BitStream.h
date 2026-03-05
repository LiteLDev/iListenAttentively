#pragma once
#include <mc/_HeaderOutputPredefine.h>
#include <optional>

namespace RakNet {

class BitStream {
public:
    uint                   mNumberOfBitsUsed;
    uint                   mNumberOfBitsAllocated;
    uint                   mReadOffset;
    uint8*                 mData;
    bool                   mCopyData;
    std::array<uint8, 256> mStackData;

public:
    BitStream()
    : mNumberOfBitsUsed(0),
      mNumberOfBitsAllocated(2048),
      mReadOffset(0),
      mData(nullptr),
      mCopyData(true),
      mStackData() {
        mStackData.fill(0);
        mData = mStackData.data();
    }
    ~BitStream() {
        if (this->mCopyData && this->mNumberOfBitsAllocated > 2048) {
            free(this->mData);
        }
    }
    BitStream& operator=(BitStream const&) = default;
    BitStream(BitStream const&)            = default;

    template <class T>
    std::optional<T> Read() {
        std::array<uchar, sizeof(T)> buffer{};
        if (ReadBits(buffer.data(), sizeof(T) * 8, true)) {
            std::ranges::reverse(buffer);
            return std::bit_cast<T>(buffer);
        }
        return std::nullopt;
    }
    template <class T>
    void Write(T value) {
        std::array<uchar, sizeof(T)> buffer{std::bit_cast<decltype(buffer)>(value)};
        std::ranges::reverse(buffer);
        WriteBits(buffer.data(), sizeof(T) * 8, true);
    }
    void IgnoreBits(uint32 numberOfBits) { mReadOffset += numberOfBits; }
    void IgnoreBytes(uint numberOfBytes) { IgnoreBits(numberOfBytes << 3); }

public:
    MCAPI void AddBitsAndReallocate(uint numberOfBitsToWrite);
    MCAPI BitStream(uchar* _data, uint lengthInBytes, bool _copyData);
    MCAPI bool Read(char* outByteArray, uint numberOfBytes);
    MCAPI bool ReadAlignedBytes(uchar* inOutByteArray, uint numberOfBytesToRead);
    MCAPI bool ReadAlignedVar16(char* inOutByteArray);
    MCAPI bool ReadAlignedVar32(char* inOutByteArray);
    MCAPI bool ReadBits(uchar* inOutByteArray, uint numberOfBitsToRead, bool alignBitsToRight);
    MCAPI void Write(::RakNet::BitStream* bitStream, uint numberOfBits);
    MCAPI void Write(char const* inputByteArray, uint numberOfBytes);
    MCAPI void Write0();
    MCAPI void Write1();
    MCAPI void WriteAlignedBytes(uchar const* inByteArray, uint numberOfBytesToWrite);
    MCAPI void WriteAlignedVar16(char const* inByteArray);
    MCAPI void WriteAlignedVar32(char const* inByteArray);
    MCAPI void WriteBits(uchar const* inByteArray, uint numberOfBitsToWrite, bool rightAlignedBits);
};

} // namespace RakNet
