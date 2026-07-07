// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#include "IPAddress.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include "ScopeExit.h"

namespace forge::utils {

auto IPv4Address::toString() const -> std::string {
    char buf[INET_ADDRSTRLEN];
    ::inet_ntop(AF_INET, &addr_, buf, sizeof(buf));
    return std::string{buf};
}

auto IPv4Address::fromString(std::string_view str) noexcept -> std::expected<IPv4Address, std::error_code> {
    ::in_addr addr{};
    if (auto const rc = ::inet_pton(AF_INET, std::string{str}.c_str(), &addr); rc != 1) {
        if (rc == 0) {
            return std::unexpected(makePosixErrorCode(EINVAL));
        } else {
            return std::unexpected(makePosixErrorCode(errno));
        }
    }
    return IPv4Address{addr};
}

auto IPv6Address::toString() const -> std::string {
    char buf[INET6_ADDRSTRLEN];
    ::inet_ntop(AF_INET6, &addr_, buf, sizeof(buf));
    return std::string{buf};
}

auto IPv6Address::fromString(std::string_view str) noexcept -> std::expected<IPv6Address, std::error_code> {
    ::in6_addr addr{};
    if (auto const rc = ::inet_pton(AF_INET6, std::string{str}.c_str(), &addr); rc != 1) {
        if (rc == 0) {
            return std::unexpected(makePosixErrorCode(EINVAL));
        } else {
            return std::unexpected(makePosixErrorCode(errno));
        }
    }
    return IPv6Address{addr};
}

auto IPAddress::toString() const -> std::string {
    if (auto const address = std::get_if<IPv4Address>(&storage_); address) {
        return address->toString();
    }
    if (auto const address = std::get_if<IPv6Address>(&storage_); address) {
        return address->toString();
    }
    return std::string{};
}

auto IPAddress::fromString(std::string_view str) noexcept -> std::expected<IPAddress, std::error_code> {
    if (auto const rc = IPv4Address::fromString(str); rc) {
        return rc.value();
    }
    if (auto const rc = IPv6Address::fromString(str); rc) {
        return rc.value();
    }
    return std::unexpected(makePosixErrorCode(EINVAL));
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

auto fmt::formatter<forge::utils::IPv4Address>::format(forge::utils::IPv4Address const& ip, format_context& ctx) const -> format_context::iterator {
    auto const addr = ip.native();
    char buf[INET_ADDRSTRLEN];
    ::inet_ntop(AF_INET, &addr, buf, sizeof(buf));
    return fmt::formatter<std::string_view>::format(buf, ctx);
}

auto fmt::formatter<forge::utils::IPv6Address>::format(forge::utils::IPv6Address const& ip, format_context& ctx) const -> format_context::iterator {
    auto const addr = ip.native();
    char buf[INET6_ADDRSTRLEN];
    ::inet_ntop(AF_INET6, &addr, buf, sizeof(buf));
    return fmt::formatter<std::string_view>::format(buf, ctx);
}

auto fmt::formatter<forge::utils::IPAddress>::format(forge::utils::IPAddress const& ip, format_context& ctx) const -> format_context::iterator {
    if (auto const addr = ip.toIPv4(); addr) {
        return fmt::formatter<forge::utils::IPv4Address>{}.format(*addr, ctx);
    }
    if (auto const addr = ip.toIPv6(); addr) {
        return fmt::formatter<forge::utils::IPv6Address>{}.format(*addr, ctx);
    }
    return format_to(ctx.out(), "<unknown>");
}
