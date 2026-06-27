/*
 * dua — disk usage analyzer
 *
 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>   
#include <sys/stat.h>


int main(int argc, char *argv[])
{

/* TODO:
 *   - recurse into subdirectories: total(dir) = sum(file sizes) + sum(total(subdir))
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
        char buf[4096];
        snprintf(buf, sizeof(buf), "%s/%s", path, dp->d_name);
        struct stat st;
        if (lstat(buf, &st) == -1) {
            perror(buf);
            continue;    
        }
        printf("%lld\t%s\n", (long long)st.st_size, buf);

    }

    closedir(dirp);
    return 0;
}
