# Design log

One entry per decision: what was chosen, what was rejected, and why. Failing
adversarial test cases go here too, with what they revealed.

## 2026-10-14: application owns the socket, not the endpoint

**Chose.** The socket is a plain struct (file descriptor only) held by the
application and passed into the protocol by reference. The protocol state
(`endpoint`) never holds a file descriptor or any OS resource.

**Rejected.** `endpoint` owning the socket. It would make `endpoint` a
resource-owning type with a destructor, break the "plain struct, trivially
copyable" rule, and force the link simulator to pretend to be a file descriptor.

**Why.** Side effects live at the edges. With the socket outside, `endpoint` is
a pure function of (state, incoming datagrams, time) and the simulator at
ticket 6 substitutes for the socket by matching four free-function signatures
(open, send, recv, close) rather than by faking an fd. The same `endpoint` then
runs unchanged inside a game loop, a server event loop, or a test with a fake
clock.

**Consequence for ticket 2.** The send and recv signatures are the contract
the simulator must later match. Decide those first.

## 2026-10-14: no exceptions, no RTTI in the library

**Chose.** `src/` is compiled with `-fno-exceptions -fno-rtti` (PRIVATE on the
`fletch` target). Runtime failures are values: `std::expected<T, error>` from
every fallible call. Programmer errors (precondition violated) are assertions.
Compile-time invariants (sizes, layouts, concept satisfaction) are
`static_assert`.

**Rejected.** Exceptions for socket and protocol errors. They are invisible in
signatures, they cost unwind tables on a hot path, and a game loop or an event
loop cannot sensibly catch them mid-tick.

**Why.** The protocol core is a pure function of state and input; an error is
just another output. Explicit error values also make the simulator's failure
injection a normal return path rather than a thrown one.

**Consequences.** Never call `std::expected::value()` or `std::optional::value()`
in `src/`; both throw on empty and under `-fno-exceptions` that is a hard
abort. Use `has_value()` then `operator*`. Tests and examples keep exceptions
because Catch2 needs them; the library must not throw through them.

## 2026-10-14: send checks socket family against address family

**Chose.** `udp_socket` records the family it was opened with. `send` returns
an error if the destination address's family differs, before calling `sendto`.

**Rejected.** Trusting the caller, letting `sendto` fail with `EAFNOSUPPORT`.
Same result, but an opaque errno instead of a named error at the call site.
