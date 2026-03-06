#pragma once
#include <ll/api/memory/Memory.h>
#include <ll/api/utils/SystemUtils.h>

namespace ila::inline utils::memory_utils {

namespace internal {

struct Address {
public:
    uintptr_t mAddress;

public:
    explicit Address(uintptr_t address) : mAddress(address) {}
    explicit Address(void* address) : mAddress(reinterpret_cast<uintptr_t>(address)) {}

public:
    template <typename T>
        requires(std::is_function_v<std::remove_pointer_t<T>> || std::is_member_function_pointer_v<T>)
    T as() const {
        union {
            uintptr_t fp;
            T         t;
        } u{};
        u.fp = mAddress;
        return u.t;
    }

    template <typename T>
        requires(std::is_pointer_v<T>)
    T as() const {
        return reinterpret_cast<T>(mAddress);
    }

public:
    explicit operator uintptr_t() const { return mAddress; }
    explicit operator void*() const { return reinterpret_cast<void*>(mAddress); }
    explicit operator bool() const { return mAddress; }
};

} // namespace internal

inline internal::Address operator""_rva(uintptr_t rva) {
    return internal::Address{ll::sys_utils::getImageRange().data() + rva};
}

template <ll::FixedString signature>
inline internal::Address operator""_sig() {
    return internal::Address{ll::memory::signatureCache<signature>.view().resolve()};
}

template <ll::FixedString symbol>
inline internal::Address operator""_sym() {
    return internal::Address{ll::memory::Symbol{symbol}.view().resolve()};
}

} // namespace ila::inline utils::memory_utils