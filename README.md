# axtual common runtime

This repository contains the I30-01 common safety/state runtime and deterministic conformance
harness authorized by `AXTUAL-I30-01-P31-v0.1`.

## Build and test

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DAXTUAL_BUILD_TESTS=ON
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
cmake --build build --config Debug --target i30_01_evidence
```

The evidence target writes `build/evidence/ea-b01-i30-01.json` with the exact revision observed
when CMake configured the build. The harness is dependency-free C++20 and is intended to run on
Linux, Windows, and macOS.

For I30-02 semantic conformance, use `cmake --build build --config Debug --target i30_02_evidence`;
it writes `build/evidence/ea-b01-i30-02.json` with the four frozen VO-END obligations and the
independently executed endpoint/policy/enhancement/self-loop cases.

## Scope

The common runtime models semantic truth, safety latches, explicit source binding, bounded queues,
finite recovery, and generation-aware stale-event rejection. Platform media APIs, drivers, UI,
public transport endpoints, and product policy are intentionally outside this slice.

## C++ style

Follow [`AGENTS.md`](AGENTS.md), which records the AXTP C++ style standard supplied for this
repository: four spaces, 100 columns, K&R braces, lower-snake-case files, explicit ownership,
controlled includes, and no platform dependencies in common public headers.
