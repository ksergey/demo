// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include <cstdint>
#include <expected>
#include <system_error>

#include <linux/if_packet.h>
#include <netdb.h>

#include "IPAddress.h"
#include "PosixError.h"

namespace forge {
namespace utils {

/// Error category for posix errors
struct GAIErrorCategory final : public std::error_category {
    constexpr GAIErrorCategory() noexcept = default;

    /// \see std::error_category
    auto name() const noexcept -> char const* override {
        return "GAIError";
    }

    /// \see std::error_category
    auto message(int error) const -> std::string override {
        return ::gai_strerror(error);
    }
};

/// Return const reference to GAIErrorCategory
[[nodiscard]] constexpr auto getGAIErrorCategory() noexcept -> std::error_category const& {
    static GAIErrorCategory errorCategory;
    return errorCategory;
}

/// Return ErrorCode with posix error
[[nodiscard]] inline auto makeGAIErrorCode(int ec) noexcept -> std::error_code {
    return std::error_code{ec, getGAIErrorCategory()};
}

class SocketAddress {
private:
    union Storage {
        ::sockaddr sa;
        ::sockaddr_in v4;
        ::sockaddr_in6 v6;
        ::sockaddr_ll raw;

        constexpr Storage() noexcept : sa{} {
            sa.sa_family = AF_UNSPEC;
        }

        constexpr Storage(sockaddr_in const& addr) : v4{addr} {}

        constexpr Storage(sockaddr_in6 const& addr) : v6{addr} {}

        constexpr Storage(sockaddr_ll const& addr) : raw{addr} {}
    } storage_;

public:
    /// Construct unitialized socket address
    SocketAddress() = default;

    /// Construct address from sockaddr_in struct (IPv4 address)
    SocketAddress(sockaddr_in const& addr) noexcept : storage_{addr} {}

    /// Construct address from sockaddr_in6 struct (IPv6 address)
    SocketAddress(sockaddr_in6 const& addr) noexcept : storage_{addr} {}

    SocketAddress(sockaddr_ll const& addr) noexcept : storage_{addr} {}

    /// Construct from IPAddress and port
    SocketAddress(IPAddress const& addr, std::uint16_t port) {
        if (addr.isIPv4()) {
            sockaddr_in sa{};
            sa.sin_family = AF_INET;
            sa.sin_addr = addr.asIPv4().native();
            sa.sin_port = hton(port);
            storage_ = Storage{sa};
        } else if (addr.isIPv6()) {
            sockaddr_in6 sa{};
            sa.sin6_family = AF_INET6;
            sa.sin6_addr = addr.asIPv6().native();
            sa.sin6_port = hton(port);
            storage_ = Storage{sa};
        } else {
            throw std::system_error{makePosixErrorCode(EINVAL), "empty address"};
        }
    }

    /// Return address family
    [[nodiscard]] auto family() const noexcept -> int {
        return storage_.sa.sa_family;
    }

    /// Returns data usable inside sockets funcs
    [[nodiscard]] auto data() const noexcept -> ::sockaddr const* {
        return &storage_.sa;
    }

    /// \overload
    [[nodiscard]] auto data() noexcept -> ::sockaddr* {
        return &storage_.sa;
    }

    /// Returns size of sockaddr
    [[nodiscard]] auto size() const noexcept -> ::socklen_t {
        switch (this->family()) {
        case AF_INET: return sizeof(::sockaddr_in);
        case AF_INET6: return sizeof(::sockaddr_in6);
        case AF_PACKET: return sizeof(::sockaddr_ll);
        default: return 0;
        }
    }

    /// Returns max size of socket address storage
    [[nodiscard]] static constexpr auto maxSize() noexcept -> socklen_t {
        return sizeof(storage_);
    }

    /// Update size of internal structs
    [[nodiscard]] auto resize(socklen_t size) -> std::expected<void, std::error_code> {
        if (sizeof(storage_) < size) {
            return std::unexpected(makePosixErrorCode(EINVAL));
        }
        return {};
    }

    /// Get IP address or EINVAL error on error
    [[nodiscard]] auto getIPAddress() const noexcept -> std::expected<IPAddress, std::error_code> {
        switch (this->family()) {
        case AF_INET: return IPAddress{storage_.v4.sin_addr};
        case AF_INET6: return IPAddress{storage_.v6.sin6_addr};
        default: return std::unexpected(makePosixErrorCode(EINVAL));
        }
    }

    /// Get port number or EINVAL error on error
    [[nodiscard]] auto getPort() const noexcept -> std::expected<std::uint16_t, std::error_code> {
        switch (this->family()) {
        case AF_INET: return ntoh(storage_.v4.sin_port);
        case AF_INET6: return ntoh(storage_.v6.sin6_port);
        default: return std::unexpected(makePosixErrorCode(EINVAL));
        }
    }

    /// Format address to string
    [[nodiscard]] auto toString() const -> std::string;

    /// Return SocketAddress or gai error
    [[nodiscard]] static auto getFromHostPort(char const* host, char const* port, int flags = 0) noexcept -> std::expected<SocketAddress, std::error_code>;
};

} // namespace utils

using namespace ::forge::utils;

} // namespace forge
