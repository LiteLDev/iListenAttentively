#pragma once
#include <ll/api/memory/Memory.h>
#include <ll/api/memory/Signature.h>
#include <ll/api/memory/Symbol.h>
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
    [[nodiscard]] T as() const noexcept {
        return std::bit_cast<T>(mAddress);
    }

    template <typename T>
        requires(std::is_member_pointer_v<T>)
    [[nodiscard]] auto as(ll::traits::function_traits<T>::class_type& self) const {
        return [this, &self](auto&&... args) -> decltype(auto) {
            return (self.*as<T>())(std::forward<decltype(args)>(args)...);
        };
    }

    template <typename T>
        requires(std::is_pointer_v<T>)
    [[nodiscard]] T as() const noexcept {
        return reinterpret_cast<T>(mAddress);
    }

    template <typename T, typename... Args>
        requires(
            (std::is_function_v<std::remove_pointer_t<T>> || std::is_member_function_pointer_v<T>)
            && !std::is_member_pointer_v<T>
        )
    decltype(auto) call(Args&&... args) const noexcept(ll::traits::function_traits<T>::is_noexcept) {
        return as<T>()(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
        requires(std::is_member_pointer_v<T>)
    decltype(auto) call(ll::traits::function_traits<T>::class_type& self, Args&&... args) const noexcept(
        ll::traits::function_traits<T>::is_noexcept
    ) {
        return (self.*as<T>())(std::forward<Args>(args)...);
    }

    explicit operator uintptr_t() const noexcept { return mAddress; }
    explicit operator void*() const noexcept { return std::bit_cast<void*>(mAddress); }
    explicit operator bool() const noexcept { return mAddress != 0; }
};

} // namespace internal

inline internal::Address operator""_rva(uintptr_t rva) {
    return internal::Address{reinterpret_cast<uintptr_t>(ll::sys_utils::getImageRange().data()) + rva};
}

template <ll::FixedString signature>
inline internal::Address operator""_sig() {
    return internal::Address{ll::memory::signatureCache<signature>.view().resolve()};
}

template <ll::memory::FixedSymbol symbol>
inline internal::Address operator""_sym() {
    return internal::Address{symbol.view().resolve()};
}

} // namespace ila::inline utils::memory_utils