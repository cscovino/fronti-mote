# AGENTS.md — include/

- Pure C++ only: no Arduino includes, so the host tests can compile it.
- New non-trivial logic gets a matching Unity suite under `test/`.
- Constants are `kFooBar`; file-local state lives in an anonymous namespace.
