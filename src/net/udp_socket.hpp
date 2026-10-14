#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>

namespace fletch::net
{
    struct error
    {
        enum class kind : std::uint8_t
        {
            os,
            family_mismatch,
            truncated,
            bad_address
        };

        kind what;
        int os_errno = 0; // only meaningful for kind::os
    };

    struct address
    {
        enum class family : std::uint8_t
        {
            v4,
            v6
        };
        family kind = family::v4;
        std::uint16_t port = 0;
        std::array<std::uint8_t, 16> ip{};
    };

    struct udp_socket
    {
        // a socket handle
        int fd = -1;
        address::family family = address::family::v4;
    };

    struct received_data
    {
        std::size_t bytes = 0;
        address from;
    };

    std::expected<udp_socket, error> open(const address& in_address_to_bind);
    std::expected<std::size_t, error> send(udp_socket in_socket, const address& in_to,
                                           std::span<const std::byte> in_data);
    std::expected<received_data, error> recv(udp_socket in_socket, std::span<std::byte> out_data);
    void close(udp_socket in_socket);
} // namespace fletch::net
