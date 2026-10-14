# Backlog

Convention: the next ticket is always at the top, under **Next**. One ticket per
session. Stop mid-ticket with a `// TODO:` marking the next step. The last minute
of every session writes the next ticket here. Scope creep goes under **Later**.

## Next

**2. Blocking UDP sender and receiver, two terminals, bytes arrive.** 30 min.
In progress on branch `ticket-02-udp-echo`, uncommitted.

Done when: two executables under `examples/` exchange a datagram over
loopback and the receiver prints sender address and byte count. Cross-check
against `nc -u` / `nc -ul`. CI green on the PR.

State at pause (2026-10-14):
- `src/net/udp_socket.hpp`: `address`, `udp_socket` (fd + family), `error`,
  `received_data`; `open`/`send`/`recv` return `std::expected`. Done.
- `src/net/udp_socket.cpp`: `to_sockaddr` (returns storage + length),
  `from_sockaddr` (optional), `as_sockaddr` with the single NOLINT, and
  `send` with the family check. Done, builds clean.
- Stubs left: `open` (socket + bind, set fd and family, close fd on bind
  failure), `recv` (recvfrom with MSG_TRUNC, truncated → error, from_sockaddr
  nullopt → error), `close` (decide what fd == -1 does).
- Then: `examples/udp_send` and `examples/udp_recv`, add to
  `examples/CMakeLists.txt`, parse the address argument with `inet_pton`.
- `src/CMakeLists.txt` is already STATIC with `-fno-exceptions -fno-rtti`.
- Also on the branch: `.clangd`, Allman + namespace indent in `.clang-format`,
  two DESIGN.md entries (socket ownership; no exceptions; family check).

## Phase 1: something running (weeks 1 to 2)

- [x] 1. Repo, CMake presets, layout, test target, CI, hello world that builds. Delegated, 15 min.
- [ ] 2. Blocking UDP sender and receiver, two terminals, bytes arrive. 30 min.
- [ ] 3. Observe your own packets in Wireshark. 20 min.

## Phase 2: the protocol core (weeks 3 to 8)

- [ ] 4. Packet header struct with serialize and deserialize, plus unit tests including wraparound. 40 min.
- [ ] 5. Sender increments sequence numbers, receiver reports what arrived. 20 min.
- [ ] 6. Link simulator: drop a configurable percentage of outgoing packets. 30 min.
- [ ] 7. Receiver builds the 32-bit ack bitfield over recent sequences. 45 min.
- [ ] 8. Sender keeps a sliding window of unacked packets, with delivery callbacks. 45 min.
- [ ] 9. RTT estimation from acks: smoothed RTT and variance per RFC 6298. 40 min.
- [ ] 10. Retransmit on RTO expiry, with exponential backoff. 45 min.
- [ ] 11. Virtual clock and seeded RNG, so simulation tests run deterministically and fast. 60 min.

## Phase 3: measurement (weeks 9 to 11)

- [ ] 12. CSV logging of every send, ack, loss and retransmit. 25 min.
- [ ] 13. Plot script for goodput and latency percentiles. Delegated, 15 min.
- [ ] 14. Scenario runner: named link profiles, batch runs, results table. 45 min.

## Phase 4: congestion control and coroutines (weeks 12 to 18)

- [ ] 15. Simple two-mode congestion switch, measured against the clean and lossy profiles. 60 min.
- [ ] 16. Congestion controller behind a concept, two implementations swappable at runtime. 60 min.
- [ ] 17. AIMD controller in the style of RFC 5681, compared against the simple one on the same scenarios. 90 min.
- [ ] 18. Pacing, so the window is spread rather than burst. 45 min.
- [ ] 19. Replace the blocking loop with a coroutine-based event loop. 2 to 3 sittings.
- [ ] 20. Channel API over the connection: unreliable, reliable-unordered, reliable-ordered. 2 sittings.

## Phase 5: shipping (weeks 19 to 22)

- [ ] 21. Fragmentation and reassembly above MTU. 60 min.
- [ ] 22. Demo example: state sync of moving entities over a lossy link.
- [ ] 23. README with the problem, the design decisions and the graphs.
- [ ] 24. Blog post: where this design and RFC 9002 disagree, and why.

## Later

Ideas that stay here until phase 5 is done:

- Congestion control variants beyond the two-mode switch and AIMD.
- Server mesh routing: several authoritative servers handing connections and entity authority between them.
- Entity replication and interest management as a layer above the channels.
- A QUIC comparison harness.
