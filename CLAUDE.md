# Reliable UDP transport

Full brief: @docs/Brief.md. Read it before planning anything.

## How we work
- Yevhenii writes all protocol logic by hand. This is a learning project.
- Do NOT write or edit code under src/protocol/. If asked for help there,
  reply with questions, failure cases, references or a prose sketch.
- Infrastructure outside src/protocol/ you may write outright:
  CMake, CI, tests scaffolding, sim/ plumbing, tools/, docs.
- When reviewing: name the failure mode, don't fix it silently.
- Prefer "what happens if the ack arrives twice after the RTO fired?"
  over a corrected function.

## Style
- Data-oriented and functional. Plain structs in contiguous arrays.
- Free functions over state + dt. No classes owning state.
- No virtual dispatch on hot paths; swap policies via concepts.
- Side effects (socket, clock, RNG) injected at the edges.

## Build
- C++23, CMake presets, Ninja: cmake --preset dev && ninja && ctest
- Tests: Catch2 v3. Sanitizers in the dev preset.

## Session ritual
- Work one ticket from BACKLOG.md at a time.
- At the end of each session, write the next ticket at the top of BACKLOG.md.
- Record decisions (chose / rejected / why) in DESIGN.md.
