"""Point core.hooksPath at .githooks on the first build.

Git cannot clone hook configuration, so this is the only way a fresh clone
gets the hooks without an extra install step.
"""

import subprocess

Import("env")  # noqa: F821 - injected by SCons

root = env.subst("$PROJECT_DIR")


def git(*args):
    return subprocess.run(["git", "-C", root, *args], capture_output=True, text=True)


try:
    if git("rev-parse", "--git-dir").returncode != 0:
        pass  # tarball, not a clone
    elif not git("config", "--local", "--get", "core.hooksPath").stdout.strip():
        git("config", "--local", "core.hooksPath", ".githooks")
        print("git hooks: enabled (.githooks)")
except Exception as exc:  # missing git must never break a build
    print("git hooks: skipped (%s)" % exc)
