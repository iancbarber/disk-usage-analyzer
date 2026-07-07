/*
 * dua — disk usage analyzer
 *
 */
#define _DEFAULT_SOURCE   /* expose POSIX funcs (lstat) under strict -std=c11 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <math.h>
#include <sys/stat.h>

long long dir_total(const char *path);
typedef struct { 
    char *path; 
    long long apparent_size; 
    long long disk_usage; 
} Entry;

typedef struct { 
    Entry *items; 
    size_t count; 
    size_t capacity; 
} EntryList;

typedef struct { 
    long long apparent; 
    long long disk; 
} Sizes;

// recursive walker — v0.2's dir_total, upgraded to two totals in one walk
Sizes subtree_sizes(const char *path);

// append e, growing items (realloc, double capacity) when count == capacity
void append_entry(EntryList *list, Entry e);

// one Entry per immediate child of path; files via lstat, subdirs via subtree_sizes
EntryList collect_entries(const char *path);

// qsort largest-first by disk_usage
void sort_entries(EntryList *list);

// write "2.3 GB" into buf
void format_size(char *buf, size_t buflen, long long bytes);

void print_entries(const EntryList *list);
void free_entries(EntryList *list);  // every path first, then items
int main(int argc, char *argv[])
{
    const char *path = (argc > 1) ? argv[1] : ".";
    dir_total(path);
    /* format_size temp tests */
    char buf[16];
    format_size(buf, 16, 0);
    printf("%s\n", buf);
    format_size(buf, 16, 1023);
    printf("%s\n", buf);
    format_size(buf, 16, 1024);
    printf("%s\n", buf);
    format_size(buf, 16, 1048575);
    printf("%s\n", buf);
    format_size(buf, 16, 1048576);
    printf("%s\n", buf);
    format_size(buf, 16, 1536);
    printf("%s\n", buf);
    format_size(buf, 16, (long long)1024*1024*1024*1024);
    printf("%s\n", buf);
    format_size(buf, 16, 10207);
    printf("%s\n", buf);
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


void format_size(char *buf, size_t buflen, long long bytes) 
{
    char unit;
    int ctr;
    double size = bytes;

    for(ctr = 0; size >= 1024; ctr++) {
        size /= 1024; 
    } 
    switch (ctr) {
        case 0:
            unit = 'B';
            break;
        case 1:
            unit = 'K';
            break;
        case 2:
            unit = 'M';
            break;
        case 3:
            unit = 'G';
            break;
        case 4:
            unit = 'T';
            break;
        default:
            snprintf(buf, buflen, "?");
            return;
    }

    double r = round(size * 10) / 10;
    if (r > 0 && r < 10) {
        snprintf(buf, buflen, "%.1f%c", r, unit);
    } else {
        snprintf(buf, buflen, "%.0f%c", r, unit);
    }
}

