// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include <netinet/in.h>

#include <bit>
#include <cstdint>
#include <expected>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <vector>

#include "Endian.h"
#include "PosixError.h"

#include <arpa/inet.h> // TODO: to cpp

namespace forge {
namespace utils {

namespace v2 {

class IPv4Address {
private:
    ::in_addr addr_{.s_addr = INADDR_ANY};

public:
    IPv4Address() = default;

    constexpr explicit IPv4Address(::in_addr addr) noexcept : addr_{addr} {}

    constexpr explicit IPv4Address(std::uint32_t addr) noexcept : addr_{.s_addr = hton(addr)} {}

    [[nodiscard]] static auto any() noexcept -> IPv4Address {
        return IPv4Address{};
    }

    [[nodiscard]] static auto loopback() noexcept -> IPv4Address {
        return IPv4Address{INADDR_LOOPBACK};
    }

    [[nodiscard]] constexpr auto toUInt() const noexcept -> std::uint32_t {
        return ntoh(addr_.s_addr);
    }

    [[nodiscard]] constexpr auto native() const noexcept -> ::in_addr {
        return addr_;
    }

    [[nodiscard]] auto toString() const -> std::string {
        char buf[INET_ADDRSTRLEN];
        ::inet_ntop(AF_INET, &addr_, buf, sizeof(buf));
        return std::string{buf};
    }

    [[nodiscard]] auto static fromString(std::string_view str) noexcept -> std::expected<IPv4Address, std::error_code> {
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

    friend constexpr auto operator<=>(IPv4Address const& a, IPv4Address const& b) noexcept {
        return a.addr_.s_addr <=> b.addr_.s_addr;
    }

    friend constexpr auto operator==(IPv4Address const& a, IPv4Address const& b) -> bool = default;
};

class IPv6Address {
private:
    ::in6_addr addr_{in6addr_any};

public:
    IPv6Address() = default;

    constexpr explicit IPv6Address(::in6_addr addr) noexcept : addr_{addr} {}

    // constexpr explicit IPv6Address(std::uint32_t addr) noexcept : addr_{.s_addr = hton(addr)} {}

    [[nodiscard]] static auto any() noexcept -> IPv6Address {
        return IPv6Address{in6addr_any};
    }

    [[nodiscard]] static auto loopback() noexcept -> IPv6Address {
        return IPv6Address{in6addr_loopback};
    }

    // [[nodiscard]] constexpr auto toUInt() const noexcept -> std::uint32_t {
    //     return ntoh(addr_.s_addr);
    // }

    [[nodiscard]] constexpr auto native() const noexcept -> ::in6_addr {
        return addr_;
    }

    [[nodiscard]] auto toString() const -> std::string {
        char buf[INET6_ADDRSTRLEN];
        ::inet_ntop(AF_INET6, &addr_, buf, sizeof(buf));
        return std::string{buf};
    }

    [[nodiscard]] auto static fromString(std::string_view str) noexcept -> std::expected<IPv6Address, std::error_code> {
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

    // friend constexpr auto operator<=>(IPv6Address const& a, IPv6Address const& b) noexcept {
    //     return a.addr_.s_addr <=> b.addr_.s_addr;
    // }

    // friend constexpr auto operator==(IPv6Address const& a, IPv6Address const& b) -> bool = default;
};

} // namespace v2

/// IPv4 address
class IPv4Address {
private:
    union Storage {
        static_assert(sizeof(::in_addr) == sizeof(std::uint32_t));

        explicit constexpr Storage(::in_addr addr) noexcept : native{addr} {}

        explicit constexpr Storage(std::uint32_t x) noexcept : addr32{x} {}

        in_addr native;
        std::uint8_t addr8[4];
        std::uint32_t addr32;
    } addr_;

public:
    constexpr IPv4Address() noexcept : addr_{0} {}

    constexpr explicit IPv4Address(std::uint32_t ip) noexcept : addr_{ip} {}

    constexpr explicit IPv4Address(in_addr const& addr) noexcept : addr_{addr} {}

    constexpr IPv4Address(std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d) noexcept
        : IPv4Address{hton((std::uint32_t(a) << 24) | (std::uint32_t(b) << 16) | (std::uint32_t(c) << 8) | (std::uint32_t(d)))} {}

    [[nodiscard]] static auto fromString(std::string_view str) noexcept -> std::expected<IPv4Address, std::error_code>;

    [[nodiscard]] static constexpr auto any() noexcept -> IPv4Address {
        return IPv4Address{};
    }

    [[nodiscard]] constexpr auto isUnspecified() const noexcept -> bool {
        return addr_.addr32 == 0;
    }

    [[nodiscard]] constexpr auto isLoopback() const noexcept -> bool {
        return addr_.addr8[0] == 127;
    }

    [[nodiscard]] static constexpr auto version() noexcept -> std::uint8_t {
        return 4;
    }

    [[nodiscard]] constexpr auto toLong() const noexcept -> std::uint32_t {
        return addr_.addr32;
    }

    [[nodiscard]] constexpr auto native() const noexcept -> in_addr {
        return addr_.native;
    }

    [[nodiscard]] auto toString() const -> std::string;

    friend constexpr auto operator<=>(IPv4Address const& a, IPv4Address const& b) noexcept {
        return a.addr_.addr32 <=> b.addr_.addr32;
    }

    friend constexpr auto operator==(IPv4Address const& a, IPv4Address const& b) noexcept {
        return a.addr_.addr32 == b.addr_.addr32;
    }
};

/// IPv6 address
class IPv6Address {
private:
    union Storage {
        static_assert(sizeof(::in6_addr) == sizeof(std::uint32_t) * 4);

        constexpr Storage(::in6_addr addr) noexcept : native(addr) {}

        constexpr Storage(std::uint8_t a0, std::uint8_t a1, std::uint8_t a2, std::uint8_t a3, std::uint8_t a4, std::uint8_t a5, std::uint8_t a6,
            std::uint8_t a7, std::uint8_t a8, std::uint8_t a9, std::uint8_t a10, std::uint8_t a11, std::uint8_t a12, std::uint8_t a13, std::uint8_t a14,
            std::uint8_t a15) noexcept
            : addr8{a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15} {}

        constexpr Storage(std::uint16_t a0, std::uint16_t a1, std::uint16_t a2, std::uint16_t a3, std::uint16_t a4, std::uint16_t a5, std::uint16_t a6,
            std::uint16_t a7) noexcept
            : addr16{a0, a1, a2, a3, a4, a5, a6, a7} {}

        constexpr Storage(std::uint32_t a0, std::uint32_t a1, std::uint32_t a2, std::uint32_t a3) noexcept : addr32{a0, a1, a2, a3} {}

        ::in6_addr native;
        std::uint8_t addr8[16];
        std::uint16_t addr16[8];
        std::uint32_t addr32[4];
    } addr_;

public:
    constexpr IPv6Address() noexcept : addr_{0, 0, 0, 0} {}

    constexpr IPv6Address(in6_addr const& addr) noexcept : addr_{addr} {}

    template <typename... Ts>
        requires(sizeof...(Ts) == 16 && std::conjunction_v<std::is_convertible<Ts, std::uint8_t>...>)
    constexpr IPv6Address(Ts... u8s) noexcept : addr_{std::uint8_t(u8s)...} {}

    template <typename... Ts>
        requires(sizeof...(Ts) == 8 && std::conjunction_v<std::is_convertible<Ts, std::uint16_t>...>)
    constexpr IPv6Address(Ts... u16s) noexcept : addr_{std::uint16_t(u16s)...} {}

    [[nodiscard]] static auto fromString(std::string_view str) noexcept -> std::expected<IPv6Address, std::error_code>;

    [[nodiscard]] constexpr auto isUnspecified() const noexcept -> bool {
        return addr_.addr32[0] == 0 && addr_.addr32[1] == 0 && addr_.addr32[2] == 0 && addr_.addr32[3] == 0;
    }

    [[nodiscard]] constexpr auto isLoopback() const noexcept -> bool {
        return addr_.addr32[0] == 0 && addr_.addr32[1] == 0 && addr_.addr32[2] == 0 && addr_.addr32[3] == hton(std::uint32_t(1));
    }

    [[nodiscard]] static constexpr auto version() noexcept -> std::uint8_t {
        return 6;
    }

    [[nodiscard]] constexpr auto native() const noexcept -> ::in6_addr {
        return addr_.native;
    }

    [[nodiscard]] auto toString() const -> std::string;

    friend constexpr auto operator<=>(IPv6Address const& a, IPv6Address const& b) noexcept {
        return std::tie(a.addr_.addr32[0], a.addr_.addr32[1], a.addr_.addr32[2], a.addr_.addr32[3]) <=>
               std::tie(b.addr_.addr32[0], b.addr_.addr32[1], b.addr_.addr32[2], b.addr_.addr32[3]);
    }

    friend constexpr auto operator==(IPv6Address const& a, IPv6Address const& b) noexcept {
        return std::tie(a.addr_.addr32[0], a.addr_.addr32[1], a.addr_.addr32[2], a.addr_.addr32[3]) ==
               std::tie(b.addr_.addr32[0], b.addr_.addr32[1], b.addr_.addr32[2], b.addr_.addr32[3]);
    }
};

class IPAddress {
private:
    struct NullIPAddress {
        [[noreturn]] auto isUnspecified() const -> bool {
            throw std::system_error{makePosixErrorCode(EINVAL), "empty address"};
        }
        [[noreturn]] auto isLoopback() const -> bool {
            throw std::system_error{makePosixErrorCode(EINVAL), "empty address"};
        }

        [[nodiscard]] static constexpr auto version() noexcept -> std::uint8_t {
            return 0;
        }

        [[nodiscard]] auto toString() const -> std::string {
            return std::string{};
        }
    };

    static_assert(std::is_trivially_copyable_v<NullIPAddress>);
    static_assert(std::is_trivially_copyable_v<IPv4Address>);
    static_assert(std::is_trivially_copyable_v<IPv6Address>);

    union Storage {
        NullIPAddress none;
        IPv4Address v4;
        IPv6Address v6;
        constexpr Storage() noexcept : none{} {}
        constexpr Storage(IPv4Address const& addr) noexcept : v4{addr} {}
        constexpr Storage(IPv6Address const& addr) noexcept : v6{addr} {}
    } storage_;

    int family_{AF_UNSPEC};

private:
    template <typename F>
    auto pick(F f) const {
        return this->isIPv4() ? f(this->asIPv4()) : this->isIPv6() ? f(this->asIPv6()) : f(this->asNull());
    }

public:
    constexpr IPAddress() = default;

    constexpr IPAddress(sockaddr const* addr) {
        if (addr == nullptr) {
            throw std::system_error{makePosixErrorCode(EINVAL), "empty address"};
        }

        family_ = addr->sa_family;
        switch (addr->sa_family) {
        case AF_INET: {
            auto v4 = std::bit_cast<sockaddr_in const*>(addr);
            storage_.v4 = IPv4Address(v4->sin_addr);
        } break;
        case AF_INET6: {
            auto v6 = std::bit_cast<sockaddr_in6 const*>(addr);
            storage_.v6 = IPv6Address(v6->sin6_addr);
        } break;
        default: throw std::system_error{makePosixErrorCode(EINVAL), "invalid address family"};
        }
    }

    constexpr IPAddress(IPv4Address const& addr) noexcept : storage_{addr}, family_{AF_INET} {}

    constexpr IPAddress(IPv6Address const& addr) noexcept : storage_{addr}, family_{AF_INET6} {}

    constexpr IPAddress(in_addr const& addr) noexcept : storage_{IPv4Address{addr}}, family_{AF_INET} {}

    constexpr IPAddress(in6_addr const& addr) noexcept : storage_{IPv6Address{addr}}, family_{AF_INET6} {}

    constexpr auto operator=(IPv4Address const& addr) noexcept -> IPAddress& {
        storage_ = Storage{addr};
        family_ = AF_INET;
        return *this;
    }

    constexpr auto operator=(IPv6Address const& addr) noexcept -> IPAddress& {
        storage_ = Storage{addr};
        family_ = AF_INET6;
        return *this;
    }

    [[nodiscard]] constexpr auto family() const noexcept -> int {
        return family_;
    }

    [[nodiscard]] constexpr auto isUnspecified() const noexcept -> bool {
        return family_ == AF_UNSPEC;
    }

    [[nodiscard]] constexpr auto isIPv4() const noexcept -> bool {
        return family_ == AF_INET;
    }

    [[nodiscard]] constexpr bool isIPv6() const noexcept {
        return family_ == AF_INET6;
    }

    [[nodiscard]] constexpr auto asNull() const -> NullIPAddress const& {
        if (!this->isUnspecified()) {
            throw std::runtime_error{"not null address"};
        }
        return storage_.none;
    }

    [[nodiscard]] constexpr auto asIPv4() const -> IPv4Address const& {
        if (!this->isIPv4()) {
            throw std::runtime_error{"can't convert address to AF_INET address"};
        }
        return storage_.v4;
    }

    [[nodiscard]] constexpr auto asIPv6() const -> IPv6Address const& {
        if (!this->isIPv6()) {
            throw std::runtime_error{"can't convert address to AF_INET6 address"};
        }
        return storage_.v6;
    }

    [[nodiscard]] auto toString() const -> std::string {
        return this->pick([](auto const& self) {
            return self.toString();
        });
    }

    friend constexpr auto operator<=>(IPAddress const& a, IPAddress const& b) noexcept -> std::partial_ordering {
        if (a.isUnspecified() || b.isUnspecified()) {
            return std::partial_ordering::unordered;
        }
        if (a.family() == b.family()) {
            return a.isIPv4() ? a.asIPv4() <=> b.asIPv4() : a.asIPv6() <=> b.asIPv6();
        } else {
            return std::partial_ordering::unordered;
        }
    }
};

struct InterfaceInfo {
    /// Interface name (i.e. wlan0)
    std::string interface;
    /// Associated ip address
    IPAddress address;
};

[[nodiscard]] auto getInterfaceAddresses(std::vector<InterfaceInfo>& storage) noexcept -> std::expected<void, std::error_code>;

} // namespace utils

using namespace ::forge::utils;

} // namespace forge
