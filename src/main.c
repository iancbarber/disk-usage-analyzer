/*
 * dua — disk usage analyzer
 *
 */
#define _DEFAULT_SOURCE   /* expose POSIX funcs (lstat) under strict -std=c11 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

long long dir_total(const char *path);

int main(int argc, char *argv[])
{
    const char *path = (argc > 1) ? argv[1] : ".";
    dir_total(path);
    return 0;
}

long long dir_total(const char *path)
{
    DIR *dirp = opendir(path);
    if (dirp == NULL) {
        perror("opendir");
        return 0;
    }
    struct dirent *dp;
    long long cur_size = 0;
    while ((dp = readdir(dirp)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
            continue;
        }
        char buf[4096];
        snprintf(buf, sizeof(buf), "%s/%s", path, dp->d_name);
        struct stat st;
        if (lstat(buf, &st) == -1) {
            perror(buf);
            continue;
        }
        if (S_ISDIR(st.st_mode)) {
            cur_size += dir_total(buf);
        } else {
            cur_size += (long long)st.st_size;
        }
    }
    printf("%lld\t%s\n", cur_size, path);
    closedir(dirp);
    return cur_size;
}
