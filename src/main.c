/*
 * dua — disk usage analyzer
 *
 * Milestone v0.1 goal: open the directory given on the command line (or ".")
 * and print each entry's name on one level (no recursion yet).
 *
 * Syscalls/functions you'll reach for:
 *   opendir(3), readdir(3), closedir(3)   — iterate directory entries
 *   lstat(2) + struct stat                — get an entry's size (st_size)
 *
 * Recurrence to keep in mind for v0.2:
 *   add size to print
 *   total(dir) = sum(file sizes in dir) + sum(total(subdir) for each subdir)
 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>   


int main(int argc, char *argv[])
{
    /* TODO v0.2
     *   3. build the full path "path/name" (watch the buffer size).
     *   4. lstat() the full path; print st_size and the name.
     */

    const char *path = (argc > 1) ? argv[1] : ".";
    DIR *dirp = opendir(path);
    if (dirp == NULL) {
        perror("opendir");
        return 1;
    }
    struct dirent *dp;
    
    while ((dp = readdir(dirp)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue; 
        printf("%s\n", dp->d_name);
    }

    closedir(dirp);
    return 0;
}
