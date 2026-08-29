---
title: '1.0a Fix macOS CI Compiler Verification'
type: 'bugfix'
created: '2026-08-29'
status: 'awaiting-operator'
baseline_revision: 'e1002db301def1fef64a289619dd2c2c18309dc5'
review_loop_iteration: 0
followup_review_recommended: true
context:
  - '/Users/bogdan/core-dance/ai/life-game/_bmad-output/project-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/_bmad-output/implementation-artifacts/spec-1-0-project-foundation-enabler.md'
warnings: []
deferred: []
operator_actions:
  - 'Merge the reviewed story commit into main.'
  - 'Push main to GitHub to trigger the required CI workflow.'
  - 'Inspect the associated GitHub Actions run and confirm both macOS jobs and both Ubuntu jobs are green.'
  - 'Confirm the macOS jobs report AppleClang 17 and the Ubuntu jobs use Clang 18 with project warnings treated as errors.'
---

## Intent

**Problem:** The committed GitHub Actions workflow cannot reach the macOS build or test steps. Both macOS jobs configure successfully, then fail because the workflow compares the compiler path recorded by CMake with a different path returned by `xcrun`, while the Ubuntu jobs pass.

**Approach:** Reproduce the failing assertion, establish whether the failure is a path-identity problem or a compiler/toolchain problem, and apply the smallest portable correction to the CI/preset boundary. Keep the AppleClang baseline check and compiler verification meaningful: the macOS jobs must still prove that the compiler selected by CMake is the approved AppleClang toolchain, without relying on an invalid host-specific path comparison.

**Evidence baseline:** GitHub Actions run [33101353778](https://github.com/bogdanbanan-coder/life-game/actions/runs/33101353778) for `main` at `532a0fb` failed in `macOS 15 Debug` and `macOS 15 Release` at `Verify CMake compiler`; Configure passed and Build/Test were skipped. `Ubuntu 24.04 Debug ASan UBSan` and `Ubuntu 24.04 Release` passed. The same assertion fails locally after `cmake --preset ci-macos-debug`: CMake records `/usr/bin/clang`, while `xcrun --find clang` resolves to `/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang`.

## Boundaries & Constraints

**Always:** Keep macOS 15 and AppleClang 17 as the required CI baseline; preserve the existing Linux Clang 18 jobs and warnings-as-errors policy; verify the compiler actually selected by CMake; keep Debug testing enabled and Release compilation gated; use committed CMake presets and workflow commands; document the proven cause and the chosen correction in the finalized spec.

**Block If:** The macOS runner cannot expose enough compiler identity to verify the approved AppleClang baseline without a product, architecture, or CI-policy decision; do not silently replace the required compiler check with an unverified success condition.

**Never:** Modify gameplay or presentation code; weaken or remove compiler/version checks; disable required jobs or tests; force-push, rewrite shared history, change product requirements, or edit `sprint-status.yaml` from the implementation worktree.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|--------------|-----------------------------|----------------|
| MACOS_REPRODUCTION | `macos-15` workflow or equivalent local preset plus the existing compiler assertion | The failure is reproduced and classified with the CMake-selected compiler identity and the `xcrun` identity/path | Record the exact differing values; do not infer the cause from `IsWindowReady`-style logs or from a successful configure alone |
| MACOS_DEBUG_CI | macOS 15 Debug checkout | AppleClang baseline, configure, compiler verification, build, and Debug CTest all pass | Fail on a wrong compiler/toolchain rather than accepting a false positive |
| MACOS_RELEASE_CI | macOS 15 Release checkout | AppleClang baseline, configure, compiler verification, and Release build pass | Fail before build if the selected compiler is not the approved AppleClang toolchain |
| LINUX_REGRESSION | Ubuntu 24.04 Debug ASan/UBSan and Release jobs | Existing Linux configure/build/test behavior remains green and uses Clang 18 | Do not solve the macOS issue by changing Linux compiler or sanitizer policy |
| PATH_VARIATION | A supported macOS runner exposes the same compiler through a different valid filesystem path or shim | Verification uses stable compiler identity/toolchain evidence rather than rejecting a valid path solely for spelling | Reject only a genuine compiler/version mismatch |

## Code Map

- `.github/workflows/ci.yml:1-93` -- macOS compiler baseline, configure, compiler identity verification, build, and test steps; the failing verification contract is here.
- `.github/scripts/verify-macos-compilers.sh` -- shared macOS verification of the CMake-selected C and C++ compiler identities against the approved AppleClang toolchain.
- `CMakePresets.json:1-137` -- committed macOS and Linux compiler selections and the CI cache variables; preserve the logical preset families and warning policy.
- `cmake/project-options.cmake` and `cmake/warnings.cmake` -- existing project warning policy that must remain unchanged by this repair.
- `_bmad-output/implementation-artifacts/spec-1-0-project-foundation-enabler.md` -- original foundation acceptance contract and the operator evidence that this repair must unblock.
- `_bmad-output/implementation-artifacts/deferred-work.md` -- update only if the repair genuinely encounters a deferred concern; do not rewrite existing entries.

## Tasks & Acceptance

**Execution:**

- `.github/workflows/ci.yml`, `.github/scripts/verify-macos-compilers.sh`, and/or `CMakePresets.json` -- reproduce the exact failed assertion, identify whether the mismatch is path spelling, compiler identity, or toolchain selection, and implement the smallest portable correction -- retain a meaningful check of the compilers CMake selected.
- CI verification command(s) -- assert the approved AppleClang compiler identity/version and the CMake-selected compiler in a way that remains valid on the `macos-15` runner -- do not replace the check with an unconditional command or a check of an unrelated compiler.
- Existing Linux workflow/presets -- run regression checks and leave Linux behavior unchanged.
- Finalized story spec -- record the root cause, changed files, verification commands, and any residual limitation; do not modify the orchestrator-owned sprint row from the story worktree.

**Acceptance Criteria:**

- Given the failing baseline at `532a0fb`, when the existing macOS compiler assertion is reproduced, then the implementation identifies the actual cause with evidence showing why the two compiler references differ and whether they refer to the same approved AppleClang toolchain.
- Given a clean checkout on `macos-15`, when the corrected `ci-macos-debug` path runs, then the AppleClang 17 baseline check, CMake configure, compiler verification, build, and `ctest --preset ci-macos-debug --output-on-failure` all pass.
- Given a clean checkout on `macos-15`, when the corrected `ci-macos-release` path runs, then the AppleClang 17 baseline check, CMake configure, compiler verification, and `cmake --build --preset ci-macos-release` all pass.
- Given the two Ubuntu jobs, when CI runs after the repair, then Ubuntu Debug ASan/UBSan and Release still configure, build, and test with Clang 18 as before.
- Given any valid path variation for the approved macOS toolchain, when verification runs, then it does not fail solely because `/usr/bin/clang` and `xcrun --find clang` have different path spellings; it does fail for a genuinely wrong compiler or version.
- Given the repair diff, when it is reviewed, then no gameplay/presentation source, required check, test, warning policy, or project requirement changes, and `git diff --check` passes.

## Design Notes

The original workflow already verifies `xcrun clang --version` contains Apple clang 17. The defect is the additional exact string comparison between the CMake cache path and `xcrun --find clang`; the correction must connect those checks to the compiler CMake actually selected rather than merely making the shell comparison return zero. Prefer stable compiler identity/version or canonical toolchain evidence over a hard-coded path, while retaining enough signal to catch an unintended compiler.

The reproduction confirms a path-identity mismatch rather than a toolchain mismatch: CMake records `/usr/bin/clang`, while `xcrun --find clang` resolves to `/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang`. On the current Darwin host, both C paths report the same AppleClang identity/version (21.0.0), while the host baseline is newer than the required CI AppleClang 17. The shared helper therefore invokes the exact C and C++ paths recorded by CMake, requires each selected compiler to report AppleClang 17, and compares its identity line with the corresponding `xcrun` compiler without comparing path spellings.

The bmad-loop implementation/review/merge gate covers the repository change and local preset-equivalent checks. After that merge, the supervisor must push `main` and treat the resulting GitHub Actions run as a separate required gate. Story 1.0 must remain `awaiting-operator` until that pushed run has green evidence for every required job.

## Verification

**Commands:**

- Reproduce the baseline assertion against `cmake --preset ci-macos-debug` and `ci-macos-release`, recording the CMake compiler value and `xcrun --find clang` value.
- `cmake --preset ci-macos-debug` -- expected: configure succeeds.
- `cmake --build --preset ci-macos-debug` -- expected: Debug build succeeds with project warnings as errors.
- `ctest --preset ci-macos-debug --output-on-failure` -- expected: all Debug tests pass.
- `cmake --preset ci-macos-release` -- expected: Release configure succeeds.
- `cmake --build --preset ci-macos-release` -- expected: Release build succeeds with project warnings as errors.
- Run the equivalent Linux preset checks available on the host, or document why the GitHub Ubuntu jobs are the required evidence.
- `git diff --check` -- expected: no whitespace errors.

Observed verification:

- The original path assertion exits 1 after `cmake --preset ci-macos-debug`; CMake reports `/usr/bin/clang` and `xcrun --find clang` reports `/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang`.
- The shared helper syntax check passes. Its logic passes for both configured macOS build directories when exercised against the local AppleClang 21 equivalent; the committed helper correctly rejects the local host because the required baseline is AppleClang 17.
- `cmake --preset ci-macos-debug`, `cmake --build --preset ci-macos-debug`, and `ctest --preset ci-macos-debug --output-on-failure` pass locally with AppleClang 21; all 167 tests pass.
- `cmake --preset ci-macos-release`, the shared compiler verification logic, and `cmake --build --preset ci-macos-release` pass locally with AppleClang 21.
- `clang-18` is not installed on this Darwin host, so the Ubuntu presets were not run locally; the committed Linux workflow and presets are unchanged and require the post-merge GitHub Actions gate.
- `git diff --check` passes.

**Supervisor post-merge gate:** Push the merged `main` commit, inspect the associated GitHub Actions run and every required job/step, and accept this repair only when both macOS jobs and both Ubuntu jobs are green. A successful CMake configure or raylib/toolchain initialization alone is insufficient evidence.

## Review Triage Log

### 2026-08-29 — Review pass
- intent_gap: 0
- bad_spec: 0
- patch: 9: (high 0, medium 3, low 6)
- defer: 0
- reject: 8
- addressed_findings:
  - `[medium]` `[patch]` Added direct AppleClang 17 checks for the CMake-selected compiler identities and matched them to the corresponding `xcrun` identities.
  - `[low]` `[patch]` Made compiler probes fail closed on command failure or empty output.
  - `[medium]` `[patch]` Added verification for the CMake-selected C++ compiler used by project targets.
  - `[low]` `[patch]` Preserved compiler paths containing `=` when extracting CMake cache values.
  - `[low]` `[patch]` Required exactly one cache entry for each compiler before invoking it.
  - `[low]` `[patch]` Centralized the duplicated Debug and Release verification logic in one CI helper.
  - `[low]` `[patch]` Documented that `baseline_revision` is the pre-change HEAD used for the review diff.
  - `[medium]` `[patch]` Recorded the reproduction, selected correction, verification evidence, and residual external gate in the finalized spec.
  - `[low]` `[patch]` Computed and recorded the follow-up review recommendation from this pass's findings.

## Auto Run Result

Status: awaiting-operator

Summary: Replaced the macOS CI exact compiler-path comparison with a shared, fail-closed helper that verifies both CMake-selected C and C++ compilers as AppleClang 17 and compares their reported identities with the corresponding `xcrun` toolchain identities. The reproduction and local preset-equivalent evidence establish that the original failure was caused by valid path spellings differing, not by selecting a different compiler. Linux workflow and preset behavior remain unchanged.

Files changed:

- `.github/workflows/ci.yml` -- calls the shared compiler verification helper from both macOS jobs.
- `.github/scripts/verify-macos-compilers.sh` -- extracts and validates the CMake-selected C and C++ compiler identities without path equality.
- `_bmad-output/implementation-artifacts/spec-1-0a-fix-macos-ci-compiler-verification.md` -- records the baseline, implementation evidence, review triage, and operator handoff.

Review findings breakdown: 9 patches applied (0 high, 3 medium, 6 low), 0 items deferred, and 8 items rejected as noise, duplicate observations, or out of scope for this correction.

Follow-up review recommendation: true. Patched findings were 0 high, 3 medium, and 6 low; score `3 × 3 + 6 = 15`.

Verification performed:

- The baseline path assertion reproduced with exit status 1 and the two differing paths recorded above.
- The shared helper passed syntax validation; its local AppleClang-21-equivalent checks passed for Debug and Release build directories, while the committed AppleClang-17 guard rejects the host's unsupported version as intended.
- macOS Debug configure/build/CTest passed locally with AppleClang 21, including 167/167 tests.
- macOS Release configure/build passed locally with AppleClang 21.
- `git diff --check` passed.
- Ubuntu Clang 18 verification was not available on the Darwin host and remains part of the required GitHub Actions gate.

Residual risks:

- This environment cannot provide the required AppleClang 17 or Ubuntu Clang 18 runner evidence.
- The reviewed commit has not been pushed; the operator must run and inspect the post-merge GitHub Actions workflow before the story is complete.
- `sprint-status.yaml` was not modified.
