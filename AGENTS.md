# Repository Instructions

## C++ Style Standard

Use `AXTP_CPP_STYLE.md` from the sibling `axtp-cpp-runtime` repository as the canonical C++
style standard, adapted only where the project name or authorized architecture differs.

- Put project symbols in `namespace axtual` or a child namespace. Interface types use an `I`
  prefix; other types do not use a redundant project prefix.
- Use four spaces, a 100-column limit, K&R braces, and no tabs. Apply the repository
  `.clang-format` file to C++ sources.
- Use `lowerCamelCase` for functions, `UpperCamelCase` for types, `kConstantName` for constants
  and new enum values, and `_lowerCamelCase` for private or protected non-static data members.
- Use `lower_snake_case` for directories and C++ filenames. New headers use `.hpp`; implementation
  files use `.cpp`. Prefer `#pragma once`.
- Include public headers through their complete module path. Public common headers may include only
  the C++ standard library and other public project headers; they must not leak platform or optional
  adapter dependencies.
- Keep common/core facilities header-only where practical, ownership explicit, and runtime control
  suitable for deterministic manual polling. Do not introduce hidden threads or ownership cycles.
- Concrete transports and platform integrations belong in optional adapter layers, never in common
  public headers.

When this standard and an approved Aegis package differ on architecture or scope, the Aegis package
controls behavior and authorization; the style standard continues to control presentation and naming.

