# ninja for building Bun

This is the `bun` branch of [oven-sh/ninja](https://github.com/oven-sh/ninja), a fork of
[ninja-build/ninja](https://github.com/ninja-build/ninja). It is upstream `master` plus the changes listed
below, and it is what Bun's build system prefers to run. Bun also builds with a stock ninja, only slower: every
change here has to keep that true (see "Compatibility").

## Changes relative to upstream

| Change | Why | Upstream |
| --- | --- | --- |
| `early_output_prefix` edge binding: a running command announces, on its output, that one of its outputs is complete; ninja releases that output to the edges consuming it while the command keeps running. | `rustc` writes a crate's `.rmeta` (all that dependent crates need) long before it has generated the `.rlib`. With this, a crate is one edge and one `rustc` process and its dependents still start as early as they do under cargo. Without it they wait for the whole compile. | Requested in [ninja-build/ninja#2286](https://github.com/ninja-build/ninja/issues/2286); not yet proposed as a pull request. |
| `.github/workflows/bun.yml`, this directory | Builds and publishes the binaries. | Fork only. |

The feature is documented in `doc/manual.asciidoc` under `early_output_prefix` and tested in
`src/subprocess_test.cc` and `src/build_test.cc`.

## Compatibility

A build file written for this branch must load and build correctly under a stock ninja.
`early_output_prefix` satisfies that when it is set on the `build` statement (stock ninja accepts unknown
variables there, and rejects them on a `rule`): a stock ninja ignores it and releases all of an edge's
outputs when the command exits. A generator should detect the feature by behaviour rather than by version —
run a one-edge build whose command prints the prefix and see whether ninja removed the line from the output.

## Keeping the branch up to date

`bun` is never rebased or force-pushed. To pick up upstream changes, merge upstream `master` into it:

```sh
git fetch https://github.com/ninja-build/ninja master
git merge FETCH_HEAD
```

Our changes stay as ordinary commits on the branch; `git log upstream/master..bun` lists them. Changes that
are useful to everyone should also be proposed upstream, and the table above updated when they land.

## Binaries

`.github/workflows/bun.yml` builds and tests the branch on every push and pull request, for the machines that
build Bun. Every build is CMake's Release configuration (`-O3`, or `/O2` with MSVC, and `NDEBUG`) with link-time
optimization; the workflow fails if LTO was not enabled.

| Archive | Built on | Notes |
| --- | --- | --- |
| `bun-ninja-linux-x64.zip`, `bun-ninja-linux-aarch64.zip` | Alpine | statically linked against musl, so it runs on glibc and musl hosts |
| `bun-ninja-darwin-x64.zip`, `bun-ninja-darwin-aarch64.zip` | macOS | deployment target 11.0 |
| `bun-ninja-windows-x64.zip`, `bun-ninja-windows-aarch64.zip` | Windows, MSVC | static C runtime |

Each archive holds `ninja` (`ninja.exe`), `COPYING` (ninja's Apache-2.0 license) and this file as `README.md`
(what the build changes relative to upstream). The Linux archives also hold `COPYRIGHT.musl`, the notice of the
musl they link statically; it is kept in `bun/licenses/musl-COPYRIGHT`, and the workflow fails if Alpine moves to
a musl version other than the one that file was taken from.

Every commit pushed to `bun` is published as the GitHub release `bun-ninja-<first 8 hex digits of the commit>`:
the six archives and `bun-ninja.json`, which records the commit, the upstream commit it sits on, the
`ninja --version` and the sha256 of each archive. A consumer pins a release and downloads
`https://github.com/oven-sh/ninja/releases/download/bun-ninja-<sha8>/bun-ninja-<os>-<arch>.zip`, checking it
against the sha256 it pinned. Pull requests are built and tested and their archives are attached to the workflow
run, but they are not released.

Actions in the workflow are pinned to full commit SHAs, which the oven-sh organization requires. Upstream's own
workflows (`linux.yml`, `macos.yml`, `windows.yml`, `linux-musl.yml`) use version tags and cannot run here; they
are disabled in the fork's Actions settings rather than edited or deleted, so that merging upstream never
conflicts on them. `bun.yml` builds and tests the same code on every host.
