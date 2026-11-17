# Pull Request

## What changes

<!-- One or two lines: what the bear can do after this PR that it could not
before. -->

## Why

<!-- The problem being solved or the feature being delivered. -->

## How to verify

Gates — all must pass on the branch head:

```bash
pio run                # firmware builds for the board
pio run -e clean       # panel blanker builds
pio test -e native     # host tests (Unity)
scripts/test-hooks.sh  # commit-msg hook self-check
```

Hardware — flash with `pio run -t upload` (if upload fails: hold BOOT, tap
RESET, release BOOT, upload again) and watch `pio device monitor`:

- [ ] <!-- One observable step per box, in the order a reviewer will do them:
command to type, what the bear must do. Include failure paths, not just the
happy one. -->

## Docs

- [ ] README sections for every changed feature match the code (commands,
  timings, wiring, limits).
- [ ] AGENTS.md updated if a coding rule, command or hook changed.
