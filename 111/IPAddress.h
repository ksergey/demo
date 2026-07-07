// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include <netinet/in.h>

#include <bit>
#include <cstdint>
#include <expected>
#include <string_view>
#include <variant>
#include <vector>

#include <fmt/format.h>

#include "Endian.h"
#include "PosixError.h"

namespace forge {
namespace utils {

class IPv4Address {
private:
    ::in_addr addr_{.s_addr = INADDR_ANY};

public:
    IPv4Address() = default;

    constexpr explicit IPv4Address(::in_addr addr) noexcept : addr_{addr} {}

    constexpr explicit IPv4Address(std::uint32_t addr) noexcept : addr_{.s_addr = hton(addr)} {}

    constexpr IPv4Address(std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d) noexcept
        : IPv4Address{(std::uint32_t{a} << 24) | (std::uint32_t{b} << 16) | (std::uint32_t{c} << 8) | std::uint32_t{d}} {}

    /// Return 0.0.0.0 address
    [[nodiscard]] static auto any() noexcept -> IPv4Address {
        return IPv4Address{};
    }

    /// Return address which represent loopback address
    [[nodiscard]] static auto loopback() noexcept -> IPv4Address {
        return IPv4Address{INADDR_LOOPBACK};
    }

    /// Return true on address is 0.0.0.0
    [[nodiscard]] constexpr auto isUnspecified() const noexcept -> bool {
        return addr_.s_addr == 0;
    }

    /// Return true on address is loopback device
    [[nodiscard]] constexpr auto isLoopback() const noexcept -> bool {
        return (ntoh(addr_.s_addr) >> 24) == 127;
    }

    /// Get native value suitable for posix calls
    [[nodiscard]] constexpr auto native() const noexcept -> ::in_addr const& {
        return addr_;
    }

    /// Format IPv4Address to string
    [[nodiscard]] auto toString() const -> std::string;

    /// Parse IPv4Address from string
    [[nodiscard]] auto static fromString(std::string_view str) noexcept -> std::expected<IPv4Address, std::error_code>;

    friend constexpr auto operator<=>(IPv4Address const& a, IPv4Address const& b) noexcept -> std::strong_ordering {
        return a.addr_.s_addr <=> b.addr_.s_addr;
    }

    friend constexpr auto operator==(IPv4Address const& a, IPv4Address const& b) noexcept -> bool {
        return a.addr_.s_addr == b.addr_.s_addr;
    }
};

class IPv6Address {
private:
    ::in6_addr addr_{::in6_addr(IN6ADDR_ANY_INIT)};

public:
    IPv6Address() = default;

    constexpr explicit IPv6Address(::in6_addr addr) noexcept : addr_{addr} {}

    template <typename... Ts>
        requires(sizeof...(Ts) == 16 && std::conjunction_v<std::is_convertible<Ts, std::uint8_t>...>)
    constexpr IPv6Address(Ts... u8s) noexcept {
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            ((addr_.s6_addr[I] = u8s), ...);
        }(std::index_sequence_for<Ts...>{});
    }

    template <typename... Ts>
        requires(sizeof...(Ts) == 8 && std::conjunction_v<std::is_convertible<Ts, std::uint16_t>...>)
    constexpr IPv6Address(Ts... u16s) noexcept {
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            ((std::bit_cast<std::uint16_t*>(std::data(addr_.s6_addr))[I] = u16s), ...);
        }(std::index_sequence_for<Ts...>{});
    }

    [[nodiscard]] static auto any() noexcept -> IPv6Address {
        return IPv6Address{::in6_addr(IN6ADDR_ANY_INIT)};
    }

    /// Return address which represent loopback address
    [[nodiscard]] static auto loopback() noexcept -> IPv6Address {
        return IPv6Address{IN6ADDR_LOOPBACK_INIT};
    }

    /// Get native value suitable for posix calls
    [[nodiscard]] constexpr auto native() const noexcept -> ::in6_addr const& {
        return addr_;
    }

    /// Format IPv6Address to string
    [[nodiscard]] auto toString() const -> std::string;

    /// Parse IPv6Address from string
    [[nodiscard]] auto static fromString(std::string_view str) noexcept -> std::expected<IPv6Address, std::error_code>;

    friend constexpr auto operator<=>(IPv6Address const& a, IPv6Address const& b) noexcept -> std::strong_ordering {
        using std::ranges::cbegin;
        using std::ranges::cend;

        return std::lexicographical_compare_three_way(cbegin(a.addr_.s6_addr), cend(a.addr_.s6_addr), cbegin(b.addr_.s6_addr), cend(b.addr_.s6_addr));
    }

    friend constexpr auto operator==(IPv6Address const& a, IPv6Address const& b) noexcept -> bool {
        return std::ranges::equal(a.addr_.s6_addr, b.addr_.s6_addr);
    }
};

class IPAddress {
private:
    std::variant<IPv4Address, IPv6Address> storage_;

public:
    constexpr IPAddress() noexcept : storage_{IPv4Address::any()} {}

    constexpr IPAddress(IPv4Address address) noexcept : storage_{address} {}

    constexpr IPAddress(::in_addr addr) noexcept : storage_{IPv4Address{addr}} {}

    constexpr IPAddress(IPv6Address address) noexcept : storage_{address} {}

    constexpr IPAddress(::in6_addr addr) noexcept : storage_{IPv6Address{addr}} {}

    /// Return true on stored IPv4 address
    [[nodiscard]] constexpr auto isIPv4() const noexcept -> bool {
        return std::holds_alternative<IPv4Address>(storage_);
    }

    /// Convert to IPv4Address if suitable
    [[nodiscard]] constexpr auto toIPv4() const noexcept -> IPv4Address const* {
        return std::get_if<IPv4Address>(&storage_);
    }

    /// Return true on stored IPv6 address
    [[nodiscard]] constexpr auto isIPv6() const noexcept -> bool {
        return std::holds_alternative<IPv6Address>(storage_);
    }

    /// Convert to IPv6Address if suitable
    [[nodiscard]] constexpr auto toIPv6() const noexcept -> IPv6Address const* {
        return std::get_if<IPv6Address>(&storage_);
    }

    /// Format IPAddress to string
    [[nodiscard]] auto toString() const -> std::string;

    /// Parse IPAddress from string
    [[nodiscard]] auto static fromString(std::string_view str) noexcept -> std::expected<IPAddress, std::error_code>;

    friend constexpr auto operator<=>(IPAddress const& a, IPAddress const& b) noexcept -> std::strong_ordering {
        return a.storage_ <=> b.storage_;
    }

    friend constexpr auto operator==(IPAddress const& a, IPAddress const& b) noexcept -> bool {
        return a.storage_ == b.storage_;
    }
};

struct InterfaceInfo {
    /// Interface name (i.e. wlan0)
    std::string interface;
    /// Associated ip address
    IPAddress address;
};

/// Get list of interface info
[[nodiscard]] auto getInterfaceAddresses(std::vector<InterfaceInfo>& storage) noexcept -> std::expected<void, std::error_code>;

} // namespace utils

using namespace ::forge::utils;

} // namespace forge

/// formatter for IPv4Address
template <>
struct fmt::formatter<forge::utils::IPv4Address> : fmt::formatter<std::string_view> {
    auto format(forge::utils::IPv4Address const& ip, format_context& ctx) const -> format_context::iterator;
};

/// formatter for IPv6Address
template <>
struct fmt::formatter<forge::utils::IPv6Address> : fmt::formatter<std::string_view> {
    auto format(forge::utils::IPv6Address const& ip, format_context& ctx) const -> format_context::iterator;
};

/// formatter for IPAddress
template <>
struct fmt::formatter<forge::utils::IPAddress> {
    constexpr auto parse(format_parse_context& ctx) -> format_parse_context::iterator {
        return ctx.begin();
    }
    auto format(forge::utils::IPAddress const& ip, format_context& ctx) const -> format_context::iterator;
};
