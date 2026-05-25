# AGENTS

This repository is a C++17 HTTP server module with a static library target,
examples, and tests. Keep agent work focused on `http-server-module-cpp`.

## Always Follow

- Keep diffs minimal and focused.
- Do not refactor or apply style changes beyond the lines you directly touch.
- Use Conventional Commits: `type(scope): summary`.
- Commit headers must be in English and include a descriptive body.

## Codebase Discovery

For non-trivial codebase investigation, architecture questions, cross-file edits,
refactors, call chains, or unknown implementation locations, use Codebase Memory
before Grep/Glob/Read/LSP.

Preferred sequence:
`index_status` -> `search_graph` / `trace_path` / `get_code_snippet` -> targeted Read/LSP.

Grep/Glob are fallback or precision-confirmation tools, not first-pass
architecture discovery.

## Project Layout

- `src/` is the only project code root. Do not create or restore a top-level
  `include/` directory.
- Public headers live under `src/http_server/...`, with the umbrella header at
  `src/http_server.hpp`.
- Keep each compiled implementation beside its matching header:
  `src/http_server/http_server_module.{hpp,cpp}`,
  `src/http_server/backend/simple_web_server_backend.{hpp,cpp}`,
  and `src/http_server/utils/request_id.{hpp,cpp}`.
- New `.hpp` and `.cpp` pairs should follow the same side-by-side pattern inside
  the nearest `src/http_server/...` subdirectory.
- The CMake build include root is `src`, so consumers should continue using
  `#include <http_server.hpp>` or `#include <http_server/...>`.

## Include Policy

- Do not use `../` in `#include` directives.
- Include project headers with forward paths from the `src` include root, for
  example `#include "http_server/route.hpp"`.
- Prefer `src/http_server.hpp` for examples and normal consumer-facing code.
- Leaf headers may be included directly when they are part of the public API, but
  they must remain self-contained.
- Do not expose backend implementation details or vendor paths through public
  headers.

## File Naming

- If a file contains exactly one primary class or struct, the file basename must
  match that type name in `CamelCase` / `PascalCase`, for example
  `HttpServerModule.hpp` for `HttpServerModule`.
- Keep matching `.hpp` and `.cpp` basenames identical when a type has a compiled
  implementation.
- If a file contains multiple classes, helpers, macros, umbrella includes, or
  utility code, use `snake_case`.

## Detailed Playbooks

Read the matching file in `guides/` when the task needs more detail:

- `guides/README.md` - index of available agent instructions.
- `guides/cpp_style.md` - naming, file naming, comments, and formatting rules.
- `guides/header-impl.md` - `.hpp` / `.ipp` / `.tpp` ownership and
  include-structure rules.
- `guides/concurrency.md` - thread-safety contracts, callback dispatch, mutex
  ordering, and shutdown invariants.
- `guides/build.md` - submodules, configure, build, and test flow.
- `guides/commits.md` - commit message rules.

## Repository Setup

Before configuring or building, initialize submodules:

```bash
git submodule update --init --recursive
```

## Build And Test

Use the existing CMake options and targets. For a normal local check:

```bash
cmake --build build
```

When changing public include structure, build tests and examples if feasible.
