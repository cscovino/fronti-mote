# AGENTS.md — src/

- Keep `loop()` non-blocking: no `delay()` outside `setup()`, no busy waits.
- Logic that can run off-device goes in `include/`, covered by a Unity test.
- Constants are `kFooBar`; file-local state lives in an anonymous namespace.
- Use `LED_BUILTIN`; the LED is active LOW.
