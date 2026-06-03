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
