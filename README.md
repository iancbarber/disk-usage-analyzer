# dua — disk usage analyzer

A small command-line tool that recursively scans a directory and reports which
files and subdirectories are using the most space — a focused reimplementation
of the core of `du` / `ncdu`, written in C.

```
$ dua ~/Downloads
  4.2 GB  ~/Downloads
  3.1 GB  ├── videos
  812 MB  ├── archive.zip
  240 MB  ├── images
   ...
```

## Status

🚧 In development. See the [roadmap](#roadmap) for current milestone.

## Build

```sh
make           # builds ./dua
make asan      # builds with AddressSanitizer for debugging memory bugs
make clean
```

Requires a C compiler (`cc`/`gcc`/`clang`) and `make`. POSIX / Linux / macOS.

## Usage

```sh
dua [PATH]     # analyze PATH (defaults to current directory)
```

## Roadmap

Built in milestones, smallest-shippable-thing first:

- **v0.1 — list one level.** Open a directory, list its entries with each
  file's size. (`opendir`, `readdir`, `lstat`)
- **v0.2 — recurse.** Descend into subdirectories and accumulate a true total
  size per directory. (structural recursion over the filesystem tree)
- **v0.3 — rank & format.** Sort entries largest-first; print human-readable
  sizes (KB/MB/GB). (`qsort`, dynamic arrays)
- **v0.4 — flags.** `--depth`, `--threshold`, sort order, `--help`. (`getopt`)
- **v0.5 — edge cases.** Don't follow symlinks (and detect cycles), survive
  permission-denied directories, de-duplicate hardlinks.
- **v1.0 — polish.** Sample output, basic tests, clean README.
- **v2.0 — performance (stretch).** Parallel traversal with threads; benchmark
  against v1.0 and measure the speedup.

## Why this project

A first systems project that leans on recursion over real (not toy) data, uses
real filesystem syscalls, and has a clean path into performance work. A planned
follow-up is a port to Zig for a side-by-side comparison.

## License

MIT
