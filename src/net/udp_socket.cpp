#include "udp_socket.hpp"

#include <cerrno>
#include <cstring>
#include <optional>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

namespace
{
    // The socket API takes sockaddr*; this cast is how it is meant to be used.
    const sockaddr* as_sockaddr(const sockaddr_storage& s)
    {
        // NOLINTBEGIN
        return reinterpret_cast<const sockaddr*>(&s);
        // NOLINTEND
    }

    struct to_sockaddr_return
    {
        sockaddr_storage storage{};
        socklen_t len = 0;
    };

    to_sockaddr_return to_sockaddr(const fletch::net::address& in_address)
    {
        to_sockaddr_return result{};
        switch (in_address.kind)
        {
        case fletch::net::address::family::v4:
        {
            sockaddr_in sin{};
            result.len = sizeof(sin);
            sin.sin_family = AF_INET;
            sin.sin_port = htons(in_address.port);
            std::memcpy(&sin.sin_addr, in_address.ip.data(), 4);
            std::memcpy(&result.storage, &sin, sizeof(sin));
            break;
        }
        case fletch::net::address::family::v6:
        {
            sockaddr_in6 sin6{};
            result.len = sizeof(sin6);
            sin6.sin6_family = AF_INET6;
            sin6.sin6_port = htons(in_address.port);
            std::memcpy(&sin6.sin6_addr, in_address.ip.data(), 16);
            std::memcpy(&result.storage, &sin6, sizeof(sin6));
            break;
        }
        }
        return result;
    }

    std::optional<fletch::net::address> from_sockaddr(const sockaddr_storage& in_storage,
                                                      socklen_t in_socklen)
    {
        fletch::net::address result;

        switch (in_storage.ss_family)
        {
        case AF_INET:
        {
            if (in_socklen < sizeof(sockaddr_in))
            {
                return std::nullopt;
            }

            sockaddr_in sin{};
            std::memcpy(&sin, &in_storage, sizeof(sin));
            result.kind = fletch::net::address::family::v4;
            result.port = ntohs(sin.sin_port);
            std::memcpy(result.ip.data(), &sin.sin_addr, 4);
            break;
        }
        case AF_INET6:
        {
            sockaddr_in6 sin6{};
            std::memcpy(&sin6, &in_storage, sizeof(sin6));
            result.kind = fletch::net::address::family::v6;
            result.port = ntohs(sin6.sin6_port);
            std::memcpy(result.ip.data(), &sin6.sin6_addr, 16);
            break;
        }
        default:
            return std::nullopt;
        }

        return result;
    }

} // namespace
namespace fletch::net
{

    std::expected<udp_socket, error> open(const address& in_address_to_bind)
    {
        udp_socket new_socket;

        auto [storage, len] = to_sockaddr(in_address_to_bind);

        return new_socket;
    }

    std::expected<std::size_t, error> send(udp_socket in_socket, const address& in_to,
                                           std::span<const std::byte> in_data)
    {
        if (in_socket.family != in_to.kind)
        {
            return std::unexpected(error{error::kind::family_mismatch});
        }

        auto [storage, len] = to_sockaddr(in_to);
        ssize_t n =
            ::sendto(in_socket.fd, in_data.data(), in_data.size(), 0, as_sockaddr(storage), len);
        if (n < 0)
        {
            return std::unexpected(error{error::kind::os, errno});
        }

        return static_cast<std::size_t>(n); // success wraps automatically
    }

    std::expected<received_data, error> recv(udp_socket in_socket, std::span<std::byte> out_data)
    {
        received_data data;
        return data;
    }

    void close(udp_socket in_socket) {}
} // namespace fletch::net
