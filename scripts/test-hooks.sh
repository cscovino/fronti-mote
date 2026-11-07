#!/usr/bin/env bash
# Self-check for .githooks/commit-msg: valid headers pass, invalid ones fail.
set -euo pipefail

cd "$(dirname "$0")/.."
tmp=$(mktemp)
trap 'rm -f "$tmp"' EXIT

failed=0
check() { # check <expected-exit> <message>
  printf '%s\n' "$2" >"$tmp"
  if .githooks/commit-msg "$tmp" 2>/dev/null; then got=0; else got=1; fi
  if [ "$got" -ne "$1" ]; then
    printf 'FAIL (exit %s, want %s): %s\n' "$got" "$1" "$2" >&2
    failed=1
  fi
}

check 0 "🐛 fix(parser): handle CRLF"
check 0 "✨️ feat: add hooks" # VS-16 byte appended by emoji keyboards
check 0 "📝 docs(readme): requirements"
check 0 "🚧 build(deps)!: bump platform"
check 0 "🎨 refactor(core): tidy"
check 0 "Merge branch 'main'"
check 0 "🐛 fix(parser): $(printf 'x%.0s' {1..57})" # 72 chars
check 1 "fix: no emoji"
check 1 "🐛 nope(parser): bad type"
check 1 "🐛 fix(parser):"
check 1 "🐛 fix(parser) add hooks"
check 1 "🐛 fix(parser): $(printf 'x%.0s' {1..58})" # 73 chars

[ "$failed" -eq 0 ] && echo "commit-msg: all checks passed"
exit "$failed"
