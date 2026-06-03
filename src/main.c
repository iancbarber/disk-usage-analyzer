/*
 * dua — disk usage analyzer
 *
 * Milestone v0.1 goal: open the directory given on the command line (or ".")
 * and print each entry's name and size on one level (no recursion yet).
 *
 * Syscalls/functions you'll reach for:
 *   opendir(3), readdir(3), closedir(3)   — iterate directory entries
 *   lstat(2) + struct stat                — get an entry's size (st_size)
 *
 * Recurrence to keep in mind for v0.2:
 *   total(dir) = sum(file sizes in dir) + sum(total(subdir) for each subdir)
 */

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    const char *path = (argc > 1) ? argv[1] : ".";

    /* TODO v0.1:
     *   1. opendir(path) — handle NULL (permission/not-a-dir) with perror.
     *   2. loop readdir() — skip "." and "..".
     *   3. build the full path "path/name" (watch the buffer size).
     *   4. lstat() the full path; print st_size and the name.
     *   5. closedir().
     */

    printf("dua: would analyze '%s' (not implemented yet)\n", path);
    return 0;
}
