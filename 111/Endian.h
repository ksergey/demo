// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include <concepts>
#include <cstdint>

namespace forge {
namespace utils {

/// Byte order enum
enum class ByteOrder { BigEndian, LittleEndian };

/// Return host byte order
[[nodiscard]] constexpr auto getHostByteOrder() noexcept -> ByteOrder {
    return (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) ? ByteOrder::BigEndian : ByteOrder::LittleEndian;
}

/// Convert host to network byte order
template <typename T>
    requires std::integral<T>
[[nodiscard]] constexpr auto hton(T value) noexcept -> T {
    if constexpr (getHostByteOrder() == ByteOrder::BigEndian) {
        return value;
    } else if constexpr (sizeof(T) == 8) {
        return __builtin_bswap64(value);
    } else if constexpr (sizeof(T) == 4) {
        return __builtin_bswap32(value);
    } else if constexpr (sizeof(T) == 2) {
        return __builtin_bswap16(value);
    } else {
        return value;
    }
}

/// Convert network to host byte order
template <typename T>
    requires std::integral<T>
[[nodiscard]] constexpr auto ntoh(T value) noexcept -> T {
    if constexpr (getHostByteOrder() == ByteOrder::BigEndian) {
        return value;
    } else if constexpr (sizeof(T) == 8) {
        return __builtin_bswap64(value);
    } else if constexpr (sizeof(T) == 4) {
        return __builtin_bswap32(value);
    } else if constexpr (sizeof(T) == 2) {
        return __builtin_bswap16(value);
    } else {
        return value;
    }
}

} // namespace utils

using namespace ::forge::utils;

} // namespace forge
