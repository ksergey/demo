// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#include <doctest/doctest.h>

#include <fmt/format.h>

#include "Endian.h"
#include "IPAddress.h"

namespace forge::utils {

TEST_CASE("IPAddress: Assignment") {
    REQUIRE_EQ(IPv4Address(0, 0, 0, 0).toString(), "0.0.0.0");
    REQUIRE_EQ(IPv6Address(0, 0, 0, 0, 0, 0, 0, 0).toString(), "::");

    IPAddress address = IPv4Address(127, 0, 0, 1);
    REQUIRE_EQ(address.toString(), "127.0.0.1");
}

TEST_CASE("IPAddress: Ordering") {
    REQUIRE_LT(IPv4Address(1, 1, 1, 1), IPv4Address(1, 1, 1, 2));
    REQUIRE_GT(IPv4Address(1, 1, 1, 2), IPv4Address(1, 1, 1, 1));
    REQUIRE_EQ(IPv4Address(0, 0, 0, 0), IPv4Address(0, 0, 0, 0));
    REQUIRE_EQ(IPv4Address(1, 2, 3, 4), IPv4Address(1, 2, 3, 4));

    REQUIRE_LT(IPv6Address(0, 0, 0, 0, 1, 1, 1, 1), IPv6Address(0, 0, 0, 0, 1, 1, 1, 2));
    REQUIRE_GT(IPv6Address(0, 0, 0, 0, 1, 1, 1, 2), IPv6Address(0, 0, 0, 0, 1, 1, 1, 1));
    REQUIRE_EQ(IPv6Address(0, 0, 0, 0, 0, 0, 0, 0), IPv6Address(0, 0, 0, 0, 0, 0, 0, 0));
}

TEST_CASE("IPv4Address: toLong") {
    IPv4Address v4(hton(static_cast<std::uint32_t>(0x11223344)));

    REQUIRE_EQ(v4.toString(), "17.34.51.68");
    REQUIRE_EQ(v4.toLong(), hton(static_cast<std::uint32_t>(0x11223344)));
}

TEST_CASE("List interfaces") {
    std::vector<InterfaceInfo> info;
    auto const rc = getInterfaceAddresses(info);
    REQUIRE(rc);

    for (auto const& [interface, address] : info) {
        fmt::print("interface: {}, address: {}\n", interface, address.toString());
    }
}

} // namespace forge::utils
