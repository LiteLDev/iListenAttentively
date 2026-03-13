#pragma once
#include <ll/api/reflection/Deserialization.h>
#include <ll/api/reflection/Serialization.h>
#include <mc/platform/UUID.h>

namespace mce {

template <class J, class T>
[[nodiscard]] inline ll::Expected<J> serialize(T&& uuid) noexcept
    requires(std::same_as<std::remove_cvref_t<T>, mce::UUID>)
try {
    return uuid.asString();
} catch (...) {
    return ll::makeExceptionError();
}
template <class T, class J>
[[nodiscard]] inline ll::Expected<> deserialize(T& uuid, J const& j) noexcept
    requires(std::same_as<T, mce::UUID>)
{
    if (j.is_string()) {
        if (auto result = mce::UUID::fromString((std::string const&)j); result != mce::UUID::EMPTY()) {
            uuid = result;
            return {};
        } else {
            return ll::makeStringError("invalid UUID string");
        }
    } else {
        return ll::makeStringError("field must be a string");
    }
}

} // namespace mce