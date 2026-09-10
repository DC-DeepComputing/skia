# Gerrit Upload Guidelines & Commit Standards

This guide explains how to prepare, format, commit, and upload a fuzzer bug fix and reproduction test to Gerrit for code review.

---

## 1. Clean Up & Pre-flight Checklist

Before committing:

1. **Remove Debug Instrumentation**:
   Ensure all temporary `path.dumpHex()`, `SKIA_LOG_D`, `SkDebugf`, and `#include "src/core/SkFloatBits.h"` statements added during diagnosis are removed from production code.
2. **Build System Registration**:
   If a new test file was created in `tests/`, ensure it is added to `gn/tests.gni`.
3. **Format Code**:
   Run `git clang-format` to match Skia's formatting guidelines:
   ```bash
   git clang-format origin/main
   ```
4. **Run Linter**:
   ```bash
   git cl lint
   ```

---

## 2. Git Branch Workflow

Always make changes on a dedicated topic branch branched from `origin/main`:

```bash
# Ensure main is up to date
git fetch origin main

# Create and switch to topic branch
git checkout -b fix-fuzz-b12345678 origin/main
```

---

## 3. Authoring the Commit Message

Skia commit messages must clearly articulate the root cause, chosen approach, and testing.

### Structure Template

```text
[<subsystem>] <Short imperative summary (<= 72 chars)>

<Detailed explanation of the failure mode. Explain what inputs (e.g. denormalized
floats, collinear control points, or truncated buffers) caused the crash or
assertion failure.>

<Explain the chosen solution. Document the rationale for this approach and mention
any alternative solutions considered (e.g. why upstream validation was chosen over
a local check, or how performance overhead was avoided).>

A reproduction unit test has been added to tests/<TestFile>.cpp.

Bug: b/123456789
TAG=agy
CONV=cd94a150-e6b7-4a14-a703-0c28f5d24663
```

### Mandatory Tags

- **`Bug:`**: The Buganizer bug number (`b/123456789`) or OSS-Fuzz issue (`oss-fuzz:12345`).
- **`TAG=agy`**: Mandatory tag identifying agent-authored changes in Google3 / Skia.
- **`CONV=<conversation_id>`**: Conversation ID tracking the agent interaction.
- **Placement**: All tag lines (`Bug:`, `TAG=`, `CONV=`) **must be placed at the very bottom** of the commit message, below all human-readable paragraphs.

---

## 4. Uploading to Gerrit with `git cl upload`

Skia uses Gerrit via `git cl` from `depot_tools`.

### Standard Upload Command

```bash
git cl upload -f -s --cq-dry-run
```

### Key Flags

- **`-f` (`--force`)**: Bypass interactive terminal prompts (required for agentic environments).
- **`-s` (`--send-mail`)**: Send notification email upon upload.
- **`-d` (`--cq-dry-run`)**: Trigger CQ dry-run tryjobs immediately to verify bots on all platforms (Linux, Mac, Windows, Android, iOS).
- **`-r <reviewer@google.com>`**: Directly assign the user or reviewer.
- **`--no-autocc`**: Prevent automatic CCing from watchlists if uploading an early draft.

### Presenting the CL to the User

Once `git cl upload` completes, extract the Gerrit URL (printed in the stdout, e.g. `https://skia-review.googlesource.com/c/skia/+/123456`) and present it clearly to the user for review.
