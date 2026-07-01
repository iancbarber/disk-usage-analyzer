# Dev notes — learning log

_Private working notes (not the public README). Maps each milestone to the
concepts it teaches, and logs what was learned / what was hard._

## Learning objectives (what this project is *for*)

| Milestone | New systems/C muscle |
|-----------|----------------------|
| v0.1 list one level | `opendir`/`readdir`/`closedir`, `lstat` + `struct stat`, reading `st_size` |
| v0.2 recurse | structural recursion over the fs tree; building full paths; accumulating totals |
| v0.3 rank & format | `malloc`/`realloc` dynamic arrays, `qsort` + comparator, integer→human-readable formatting |
| v0.4 flags | `getopt`/`getopt_long`, argument parsing, `--help` text |
| v0.5 edge cases | `lstat` vs `stat` (symlinks), `S_ISLNK`, `errno`/`EACCES` handling, `st_ino`/`st_dev` for hardlink dedup |
| v2.0 performance | `pthreads`, work distribution, benchmarking + measuring speedup |

Discipline throughout: run `make asan` and/or `valgrind ./dua` to catch leaks
and out-of-bounds early. The point of doing this in C is to *see* the memory
model — treat every `malloc` as owing a `free`.

## The core insight to hold

The filesystem is a **tree**: a directory is a node, a file is a leaf, and the
size of a directory is its own entries plus the recursive total of its
subdirectories. This is the same structural-recursion template from the BST /
recursive-ADT work — `total(dir) = sum(file sizes) + sum(total(subdir))`. State
that recurrence in one English sentence before writing each function.

## Session log

- 2026-06-03 — Repo scoped, scaffold created (README, Makefile, stub `main.c`,
  .gitignore). Decision: build in C (compounds into CMU 15-213), port to Zig as
  a later project. Next: implement v0.1 (open a dir, list entries + sizes).
- 2026-06-09 — **v0.1 implemented (6-day stall broken).** `opendir`/`readdir`/
  `closedir` loop printing each entry name one level; skips `.`/`..` via a
  `continue` guard; `perror("opendir")` on failure. Sizes deferred to v0.2.
  Learned this session: (1) a failing syscall should report *why* (`perror` +
  `errno`); (2) `const DIR *` is wrong — `readdir` mutates the handle, so the
  compiler warns about discarding the qualifier; `const` is for read-only data
  like `const char *path`; (3) comments should state what's LEFT, not narrate
  what the code already does. **Next: v0.2** — build full path `"path/name"`
  (watch buffer size), `lstat` it, print `st_size` next to the name.
- 2026-06-27 — **v0.1 size half done (project dark ~14 days, restarted).**
  Added `<sys/stat.h>`; `lstat(buf, &st)` per entry, `perror(buf)` + `continue`
  on failure (one bad entry must not kill the scan), print `(long long)st.st_size`.
  Learned this session: (1) **`perror` label must name the syscall that actually
  failed** — had a copy-pasted `perror("opendir")` in the `lstat` block (same
  "wrong literal" family as the morning's `0` vs `'0'` Number-of-Islands bug:
  a plausible-looking string that doesn't match the thing it describes);
  (2) **the `(long long)` cast on `st_size` is about printf, not bit-width** —
  `printf` is variadic and can't see argument types, `off_t`'s exact underlying
  type is unspecified, so cast to a known type whose specifier (`%lld`) you know;
  mismatch = UB (recurring 15-213 theme); (3) `\t` aligns to tab *stops* (every 8
  cols), not true columns — wide values overflow and go ragged; real alignment is
  a field width (`%10lld`), deferred to v0.3. Commit: `Print each file's size in
  bytes` (imperative + behavior-first, drafted by Ian after critique).
  **Next: v0.2** — recurse: `total(dir) = sum(file sizes) + sum(total(subdir))`.
- 2026-07-01 — **v0.2 recursion — the tool finally does its own name.**
  Refactored the one-level scan into `long long dir_total(const char *path)`
  that opens its own dir, and for each entry recurses on subdirs / adds
  `st_size` for files: `total(dir) = sum(file sizes) + sum(total(subdir))`.
  Prints each directory's cumulative total post-order (`du`-style, children
  before parents). The functional→C translation felt hard going in and came
  out correct on the first real draft. Learned this session: (1) **recursion
  threads on PATH STRINGS, not `DIR*` handles** — a `DIR*` is an opaque cursor
  that doesn't know its own path; each call owns its `opendir`/`closedir` and
  passes the child *path* down; (2) **`return 0`, not `1`, on `opendir` failure**
  — the return value is now a SIZE summed by the caller, so an unreadable dir
  must contribute 0 (returning 1 = "1 phantom byte"; same wrong-literal-in-a-new-
  context family as the 06-27 `perror` label); (3) **`S_ISDIR(st.st_mode)`, not
  `== S_IFDIR`** — `st_mode` packs permission bits, so use the test macro that
  masks; (4) the `.`/`..` skip is now **load-bearing** (recursing either = infinite
  descent), and `lstat` (not `stat`) also stops us following symlinks out of the
  tree; (5) **feature-test macro** — `lstat` warned "implicit declaration" on
  Linux/glibc under `-std=c11` (strict ISO C hides POSIX prototypes; macOS
  declares it unconditionally so I'd never seen it). Fix: `#define _DEFAULT_SOURCE`
  above the includes. Implicit declaration → wrong assumed prototype → **UB**, the
  same family as a printf specifier mismatch — a recurring 15-213 theme.
  **Verified, not assumed:** built a tree with a known total (20 bytes), cross-
  checked with an independent `find` sum, confirmed a permission-denied subdir
  warns + contributes 0 + doesn't crash. Meta-lesson: a "compiled clean" message
  that doesn't actually check for warnings is itself a literal-audit failure.
  **Next: v0.3** — rank largest-first + human-readable sizes (`qsort`, dynamic
  arrays, KB/MB/GB formatting).
