// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#include "IPAddress.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <algorithm>

#include "ScopeExit.h"

namespace forge::utils {

auto IPv4Address::fromString(std::string_view str) noexcept -> std::expected<IPv4Address, std::error_code> {
    in_addr addr;
    if (::inet_pton(AF_INET, std::string{str}.c_str(), &addr) != 1) {
        return std::unexpected(makePosixErrorCode(EINVAL));
    }
    return IPv4Address{addr};
}

auto IPv4Address::toString() const -> std::string {
    char buf[INET_ADDRSTRLEN] = "";
    ::inet_ntop(AF_INET, &addr_.native, buf, INET_ADDRSTRLEN);
    return std::string{buf};
}

auto fromString(std::string_view str) noexcept -> std::expected<IPv6Address, std::error_code> {
    in6_addr addr;
    if (::inet_pton(AF_INET6, std::string{str}.c_str(), &addr) != 1) {
        return std::unexpected(makePosixErrorCode(EINVAL));
    }
    return IPv6Address{addr};
}

auto IPv6Address::toString() const -> std::string {
    char buf[INET6_ADDRSTRLEN] = "";
    ::inet_ntop(AF_INET6, &addr_.native, buf, INET6_ADDRSTRLEN);
    return std::string(buf);
}

auto getInterfaceAddresses(std::vector<InterfaceInfo>& storage) noexcept -> std::expected<void, std::error_code> {
    ifaddrs* ifaddr{nullptr};
    if (::getifaddrs(&ifaddr) == -1) {
        return std::unexpected(makePosixErrorCode(errno));
    }

    ScopeExit scopeExit{[&] {
        ::freeifaddrs(ifaddr);
    }};

    storage.clear();

    for (auto ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) {
            continue;
        }

        switch (ifa->ifa_addr->sa_family) {
        case AF_INET: storage.emplace_back(ifa->ifa_name, IPAddress{std::bit_cast<sockaddr_in*>(ifa->ifa_addr)->sin_addr}); break;
        case AF_INET6: storage.emplace_back(ifa->ifa_name, IPAddress{std::bit_cast<sockaddr_in6*>(ifa->ifa_addr)->sin6_addr}); break;
        default: break;
        }
    }

    std::ranges::sort(storage, [](auto const& a, auto const& b) {
        return a.interface < b.interface;
    });

    return {};
}

} // namespace forge::utils
