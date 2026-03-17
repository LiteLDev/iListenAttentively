#pragma once
#include <mc/common/SubClientId.h>
#include <mc/network/NetworkIdentifier.h>
#include <mc/network/NetworkIdentifierWithSubId.h>
#include <type_traits>

template <>
struct std::hash<NetworkIdentifierWithSubId> {
    size_t operator()(NetworkIdentifierWithSubId const& id) const {
        return std::hash<NetworkIdentifier>{}(id.id) ^ std::hash<SubClientId>{}(id.subClientId);
    }
};

inline bool operator==(NetworkIdentifierWithSubId const& a, NetworkIdentifierWithSubId const& b) {
    return a.id == b.id && a.subClientId == b.subClientId;
}