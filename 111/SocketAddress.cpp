// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#include "SocketAddress.h"

#include <bit>
#include <utility>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <fmt/format.h>

namespace forge::utils {

class SocketAddrInfo {
private:
    addrinfo* info_{nullptr};

public:
    SocketAddrInfo(SocketAddrInfo const&) = delete;
    SocketAddrInfo& operator=(SocketAddrInfo const&) = delete;

    SocketAddrInfo() = default;

    SocketAddrInfo(addrinfo* info) noexcept : info_{info} {}

    ~SocketAddrInfo() noexcept {
        if (info_) {
            ::freeaddrinfo(info_);
        }
    }

    SocketAddrInfo(SocketAddrInfo&& other) noexcept : info_{std::exchange(other.info_, nullptr)} {}

    auto operator=(SocketAddrInfo&& other) noexcept -> SocketAddrInfo& {
        if (this != &other) {
            this->~SocketAddrInfo();
            new (this) SocketAddrInfo{std::move(other)};
        }
        return *this;
    }

    [[nodiscard]] auto info() const noexcept -> addrinfo const* {
        return info_;
    }
};

[[nodiscard]] auto getAddrInfo(char const* host, char const* port, addrinfo const* hints) noexcept -> std::expected<SocketAddrInfo, std::error_code> {
    addrinfo* info{nullptr};
    auto const error = ::getaddrinfo(host, port, hints, &info);
    if (error != 0) [[unlikely]] {
        return std::unexpected(makeGAIErrorCode(error));
    }
    return SocketAddrInfo{info};
}

auto SocketAddress::toString() const -> std::string {
    switch (this->family()) {
    case AF_UNSPEC: return "<uninitialized address>";
    case AF_INET: return fmt::format("{}:{}", IPv4Address{storage_.v4.sin_addr}.toString(), ntoh(storage_.v4.sin_port));
    case AF_INET6: return fmt::format("[{}]:{}", IPv6Address{storage_.v6.sin6_addr}.toString(), ntoh(storage_.v6.sin6_port));
    case AF_PACKET:
    default: break;
    }
    return fmt::format("<unknown address family {}>", this->family());
}

auto SocketAddress::getFromHostPort(char const* host, char const* port, int flags) noexcept -> std::expected<SocketAddress, std::error_code> {
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV | AI_PASSIVE | flags;

    auto result = getAddrInfo(host, port, &hints);
    if (!result) {
        return std::unexpected(result.error());
    }

    auto info = result.value().info();
    auto address = info->ai_addr;
    switch (address->sa_family) {
    case AF_INET: return SocketAddress{*std::bit_cast<sockaddr_in const*>(address)};
    case AF_INET6: return SocketAddress{*std::bit_cast<sockaddr_in6 const*>(address)};
    default: break;
    }
    return std::unexpected(makeGAIErrorCode(EAI_ADDRFAMILY));
}

} // namespace forge::utils
