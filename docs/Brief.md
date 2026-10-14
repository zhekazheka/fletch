# Reliable UDP Transport in C++23 — Project Brief

communcationOct 14, 2026 · @Yevhenii Tolstikov

## Goal

Build a reliable transport protocol on top of UDP in C++23, with congestion control, a deterministic link simulator, and measured results, published as a public repo with a written design rationale.

The project serves three outcomes at once:

1. **A portfolio artifact outside a game engine.** Fifteen years of netcode currently reads as "games only" on a CV. This repo is the proof that the same skills apply to transport-layer systems work, in language that defence, packet-processing, real-time media and backend employers recognise.
2. **Closing the modern C++ gap.** Coroutines, concepts and ranges get used where they genuinely fit, not as decoration. The async path is coroutine-based by the end.
3. **Interview material that holds up under questioning.** The target standard: every design decision in the protocol core can be defended on a whiteboard, a year later, without notes.
4. **A library worth keeping.** The long-term ambition is a general-purpose network library, usable for any machine-to-machine communication, that is also good enough to carry a game's netcode. That second use is where the interesting experiments live: entity replication, interest management, and eventually server mesh topologies. Nothing game-specific goes into the core, but nothing in the core may rule those out either.

That third point drives the working model below. An impressive repo the author cannot explain is worse than a modest one he can.

## Working model: hybrid, human-led

This is a learn-by-doing project. Yevhenii writes the protocol logic by hand; Claude Code plans, scaffolds, reviews and attacks it. The split is deliberate and should be held even when delegating would be faster.

| Area | Who |
| --- | --- |
| Packet header format, serialization | Yevhenii |
| Sequence numbers, ack bitfield, sliding window | Yevhenii |
| RTT estimation, RTO, retransmission logic | Yevhenii |
| Congestion controller and all tuning constants | Yevhenii |
| Coroutine event loop and channel API | Yevhenii |
| CMake, presets, directory layout, CI, test target | Claude Code |
| Ticket breakdown and session planning | Claude Code |
| Code review, design critique, adversarial test cases | Claude Code |
| Link simulator plumbing, CSV logging, plotting scripts | Claude Code |
| README structure, doc comments, diagrams | Claude Code |
| Debugging partner on failures Yevhenii cannot locate | Claude Code |

**Standing instructions for Claude Code on this repo:**

- Do not write code under `src/protocol/` unless explicitly asked. When asked for help there, respond with questions, failure cases, references or a sketch in prose, not a finished implementation.
- When reviewing, name the failure mode and let the author fix it. No silent rewrites.
- Prefer "what happens if the ack arrives twice, out of order, after the RTO fired?" over "here is the corrected function."
- Infrastructure outside `src/protocol/` can be written outright.
- At the end of each session, write the next ticket into `BACKLOG.md` so the next session starts with no decision to make.

**Session shape.** Home blocks of 45 to 120 minutes, one ticket each, on personal hardware only. Reading specs, sketching state machines, writing tickets and reviewing diffs are break-sized tasks that need no dev environment. Always stop mid-ticket with a `// TODO:` marking the next step.

## Language, toolchain and setup

C++23, Linux-first, cross-building on Windows. No framework carries the protocol logic.

| Choice | Decision |
| --- | --- |
| Language | C++23 (GCC 14+ / Clang 18+ / MSVC 19.4x) |
| Build | CMake 3.28+ with presets, Ninja generator |
| Dependencies | Open decision, see below. Minimal either way; FetchContent if any |
| Sockets | Raw BSD sockets first, blocking. Whether the async path moves to standalone Asio or stays hand-rolled is an open decision, taken at ticket 19 |
| Async model | C++20 coroutines (`co_await` on socket operations), single-threaded event loop. The awaitable layer underneath is the open decision below |
| Tests | Catch2 v3 |
| Quality | clang-format, clang-tidy, ASan + UBSan builds in CI, TSan if threads appear |
| CI | GitHub Actions, Linux Debug + Release, sanitizer job |
| Plots | Python 3 + matplotlib, in `tools/`, fully delegated |

**The socket layer is an open decision, deliberately postponed.** Raw sockets come first because the protocol is the thing being learned, and adding a library at the same time blurs where behaviour comes from. By ticket 19 the state machine is understood, and the choice gets made on evidence rather than in advance.

Criteria for that decision, recorded now so it is not made by inertia: how cleanly it integrates with C++20 coroutines; whether it drags in dependencies a potential adopter would object to; its licence; Windows and Linux parity; and whether it fights the data-oriented style by insisting on object ownership of connections. Hand-rolling an epoll and IOCP loop stays a legitimate outcome, and so does Asio.

**Layout.**

```markdown
src/protocol/   # hand-written: packets, connection, congestion, channels
src/net/        # socket wrapper, event loop
sim/            # link simulator, virtual clock, scenario runner
tests/          # Catch2 unit + deterministic simulation tests
tools/          # plotting, log analysis
examples/       # echo, file transfer, state-sync demo
BACKLOG.md      # tickets, next one always at the top
DESIGN.md       # decision log: chose, rejected, why
```

First session for Claude Code: scaffold all of the above, a hello-world target, a passing placeholder test, CI config, and confirm `cmake --preset dev && ninja && ctest` works end to end.

## Network stack and protocol scope

The protocol sits directly on UDP (IPv4 and IPv6), unicast, one socket per endpoint, client-server topology with a listening server accepting many connections. Payload MTU assumed at 1200 bytes to stay under common path MTUs without discovery.

**In scope**

- Connection handshake, keepalive and teardown, with connection ids so a client surviving a NAT rebind is at least representable
- 16-bit sequence numbers with wraparound-safe comparison
- Ack encoding: latest sequence plus a 32-bit bitfield of prior acks
- Per-packet delivery notification to the application layer (acked or considered lost)
- RTT estimation with smoothed RTT and variance, RTO calculation and exponential backoff, following RFC 6298
- Retransmission queue and a sliding send window
- Fragmentation and reassembly for payloads above MTU
- Congestion control, built in two stages: a simple two-mode throughput switch first, then AIMD in the style of RFC 5681, with the controller behind a concept so alternatives can be swapped and compared
- Pacing, so a window is not dumped as a burst
- Channels: unreliable-unordered, reliable-unordered, reliable-ordered, over one connection, with per-channel delivery guarantees chosen by the caller. Settled: delivery mode is a transport concern, replication is not

**Out of scope**

- Encryption, authentication, TLS or QUIC's crypto handshake
- NAT traversal, STUN, TURN, hole punching
- Multicast, broadcast, kernel bypass, DPDK, io\_uring
- Wire compatibility with QUIC or any existing protocol
- Path MTU discovery
- Production hardening: DoS resistance, amplification limits, formal fuzzing

**Deferred, not abandoned.** Server mesh (several authoritative servers sharing one world, handing connections and entity authority between them) is a stated long-term goal and no code for it lands before phase 5. Two cheap decisions now keep it reachable: connection identity is a connection id rather than an address and port, so a peer can be handed to a different server without reconnecting; and the packet header reserves a routing field from the first version. Entity replication and interest management sit in the same bucket, as a layer above the channels rather than inside the protocol.

The general-purpose case stays primary. If a feature only makes sense for games, it belongs in a layer on top, not in `src/protocol/`.

**Reference specifications.** RFC 768 for UDP, RFC 6298 for retransmission timers, RFC 5681 for TCP congestion control, RFC 9002 for QUIC loss recovery and congestion control. RFC 9002 is the comparison target throughout: the interesting writeup material is where this design and QUIC's disagree, and why.

## Architecture

&#91;embedded content: transport architecture · 8 components, hand-written core banded\]

The application never sees a packet: it writes to a channel, and the connection decides what gets sent, what gets retransmitted and when. The socket layer below is swappable, which is what makes the link simulator possible without touching the protocol at all.

**Design style: data-oriented and functional, not object-oriented.** State lives in plain structs held in contiguous arrays: connections, packets in flight, pending acks, channel queues. The components in the diagram are namespaces of free functions that take that state plus elapsed time and return what changed, not classes that own it. No virtual dispatch on a hot path; the congestion controller is swapped at compile time through a concept rather than through an interface.

Side effects live at the edges. Socket writes, clock reads and randomness are injected, so the protocol core is a pure function of state and input. That is not a style preference alone: it is exactly what makes the deterministic simulation and seed-reproducible failures in the testing section possible, and it is what will later allow the same core to run inside a game loop, a server process, or a test harness without changes.

## Backlog

Each ticket fits one sitting and has an obvious "done". Claude Code owns keeping this list current in `BACKLOG.md`, with the next ticket always at the top.

**Phase 1: something running (weeks 1 to 2)**

- [ ] 1\. Repo, CMake presets, layout, test target, CI, hello world that builds. Delegated, 15 min.
- [ ] 2\. Blocking UDP sender and receiver, two terminals, bytes arrive. 30 min.
- [ ] 3\. Observe your own packets in Wireshark. 20 min.

**Phase 2: the protocol core (weeks 3 to 8)**

- [ ] 4\. Packet header struct with serialize and deserialize, plus unit tests including wraparound. 40 min.
- [ ] 5\. Sender increments sequence numbers, receiver reports what arrived. 20 min.
- [ ] 6\. Link simulator: drop a configurable percentage of outgoing packets. 30 min.
- [ ] 7\. Receiver builds the 32-bit ack bitfield over recent sequences. 45 min.
- [ ] 8\. Sender keeps a sliding window of unacked packets, with delivery callbacks. 45 min.
- [ ] 9\. RTT estimation from acks: smoothed RTT and variance per RFC 6298. 40 min.
- [ ] 10\. Retransmit on RTO expiry, with exponential backoff. 45 min.
- [ ] 11\. Virtual clock and seeded RNG, so simulation tests run deterministically and fast. 60 min.

**Phase 3: measurement (weeks 9 to 11)**

- [ ] 12\. CSV logging of every send, ack, loss and retransmit. 25 min.
- [ ] 13\. Plot script for goodput and latency percentiles. Delegated, 15 min.
- [ ] 14\. Scenario runner: named link profiles, batch runs, results table. 45 min.

**Phase 4: congestion control and coroutines (weeks 12 to 18)**

- [ ] 15\. Simple two-mode congestion switch, measured against the clean and lossy profiles. 60 min.
- [ ] 16\. Congestion controller behind a concept, two implementations swappable at runtime. 60 min.
- [ ] 17\. AIMD controller in the style of RFC 5681, compared against the simple one on the same scenarios. 90 min.
- [ ] 18\. Pacing, so the window is spread rather than burst. 45 min.
- [ ] 19\. Replace the blocking loop with a coroutine-based event loop. 2 to 3 sittings.
- [ ] 20\. Channel API over the connection: unreliable, reliable-unordered, reliable-ordered. 2 sittings.

**Phase 5: shipping (weeks 19 to 22)**

- [ ] 21\. Fragmentation and reassembly above MTU. 60 min.
- [ ] 22\. Demo example: state sync of moving entities over a lossy link.
- [ ] 23\. README with the problem, the design decisions and the graphs.
- [ ] 24\. Blog post: where this design and RFC 9002 disagree, and why.

The only commitment that counts for the first fortnight is one ticket per week. Beating it is fine; the bar is set where failure is not available.

## Testing and measurement

Most tests run without touching a real socket. A virtual clock plus a seeded RNG means two endpoints talk in-process, a sixty-second lossy scenario finishes in milliseconds, and a failure reproduces exactly from its seed. This is the single highest-value design decision in the project and it should land early, at ticket 11, before the congestion work.

**Link profiles** used across all scenarios:

| Profile | Loss | One-way latency | Jitter | Reorder | Bandwidth |
| --- | --- | --- | --- | --- | --- |
| LAN | 0% | 1 ms | 0 ms | 0% | unlimited |
| Good broadband | 0.5% | 20 ms | 3 ms | 0.1% | 50 Mbit/s |
| Mobile | 2% | 60 ms | 25 ms | 1% | 5 Mbit/s |
| Transatlantic | 1% | 90 ms | 10 ms | 0.5% | 20 Mbit/s |
| Hostile | 10% | 150 ms | 50 ms | 5% | 1 Mbit/s |
| Bottleneck | 0% | 20 ms | 2 ms | 0% | 256 kbit/s |

The simulator also injects duplicates, burst loss windows and a full blackout period, since those break naive retransmission logic in ways uniform random loss never does.

**Metrics** recorded per run: goodput, latency p50 / p95 / p99, retransmission ratio, header overhead as a share of bytes sent, recovery time after a loss burst, and queue depth over time. Every congestion control change gets rerun across all six profiles, and the comparison table goes in the README.

**Test layers**

1. Unit: header round-trip, sequence wraparound comparison, ack bitfield construction, window bookkeeping.
2. Simulation: full connection over a profile, asserting delivery guarantees per channel type.
3. Adversarial: the cases worth asking Claude Code to generate, including duplicate acks after an RTO fired, acks arriving for packets already considered lost, sequence wraparound mid-flight, a receiver that vanishes for ten seconds and returns, and reordering deep enough to fall outside the ack bitfield.

A failing adversarial case is the most valuable output of a session. Record it in `DESIGN.md` with what it revealed.

## Definition of done

The project ships when a stranger can clone it, build it in one command, and understand the reasoning behind it without asking questions.

- Public repository, `cmake --preset dev && ninja && ctest` green on a clean machine, CI passing on Linux Debug, Release and sanitizers
- The protocol core implements everything listed in scope, with channels working over a single connection
- Deterministic simulation suite covering all six link profiles, reproducible from seeds
- A README that opens with the problem, states the design decisions, and shows the goodput and latency graphs comparing the two congestion controllers
- `DESIGN.md` carrying the decision log: what was chosen, what was rejected, why
- One demo under `examples/` that is visibly interesting: entity state sync over the Hostile profile, side by side with and without the transport's loss handling
- A published writeup on where this design and RFC 9002 part ways

**The real acceptance test**, applied before calling it finished: pick any file under `src/protocol/` at random, and explain on paper what it does, why it is structured that way, what was tried first, and how it behaves when the network misbehaves. If any file fails that test, it gets rewritten by hand before the repo goes public.

## Constraints and working rules

- **Personal hardware, personal time.** Danish employment contracts carry broad IP clauses as a rule, so no part of this is written on work machines or during work hours.
- **Not production software.** No security, no hardening, no stability promises. The README says so plainly rather than implying otherwise.
- **Two habits only.** For the first six weeks, the daily algorithm problem and this project. Reading, the agent drills and the decision log get added once the build sessions run without effort.
- **One ticket a week is the floor**, set deliberately below capacity. Streaks are irrelevant; a missed week is not repaid, it is simply followed by a ticket.
- **Never start a session by deciding.** The last minute of every session writes the next ticket.
- **Stop mid-ticket.** Clean stopping points make restarting expensive.
- **Scope creep goes to the backlog, not into the current ticket.** The ideas worth having (congestion control variants, server mesh routing, entity replication on top of the channels, a QUIC comparison harness) are listed at the bottom of `BACKLOG.md` under Later, and stay there until phase 5 is done.
