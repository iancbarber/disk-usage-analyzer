/*
 * dua — disk usage analyzer
 *
 */
#define _DEFAULT_SOURCE   /* expose POSIX funcs (lstat) under strict -std=c11 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <math.h>
#include <sys/stat.h>
#include <unistd.h>

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
void sort_entries(EntryList *list, int (*compare_func)(const void *, const void *));
// compare func for qsort to sort entries from max to min disk_usage size
int compare_disk_desc(const void *e1, const void *e2);

// compare func for qsort to sort entries from max to min apparent_size size
int compare_apparent_desc(const void *e1, const void *e2); 

// write "2.3 GB" into buf
void format_size(char *buf, size_t buflen, long long bytes);

void print_entries(const EntryList *list);
void free_entries(EntryList *list);  // every path first, then items
int main(int argc, char *argv[])
{
    int ch;
    int apparent = 0;
    while ((ch = getopt(argc, argv, "a")) != -1) {
        switch (ch) {
        case 'a':
            apparent = 1;
            break;
        default:
            fprintf(stderr, "Usage: ./dua DIR or ./dua -a DIR to sort by apparent size.\n");
            exit(1);
            break;
        }
    } 
    argc -= optind; 
    argv += optind;
    const char *path = (argc > 0) ? argv[0] : ".";
    EntryList list = collect_entries(path);
    sort_entries(&list, apparent ? compare_apparent_desc : compare_disk_desc);
    print_entries(&list);
    free_entries(&list);
    return 0;
}

Sizes subtree_sizes(const char *path)
{
    DIR *dirp = opendir(path);
    if (dirp == NULL) {
        perror(path);
        return (Sizes){0,0};
    }
    struct stat st_dir;
    Sizes cur_dir_size = {0, 0};
    if (lstat(path, &st_dir) == -1) {
        perror(path);
    } else {
        cur_dir_size.apparent = st_dir.st_size;
        cur_dir_size.disk = (long long)st_dir.st_blocks * 512;
    }
    struct dirent *dp;
    Sizes sizes = {.apparent=0, .disk=0}; 
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
            Sizes tmp = subtree_sizes(buf);
            sizes.apparent += tmp.apparent; 
            sizes.disk += tmp.disk; 
        } else {
            sizes.apparent += (long long)st.st_size;
            sizes.disk += (long long)st.st_blocks * 512;
        }
    }
    closedir(dirp);
    sizes.apparent += cur_dir_size.apparent;
    sizes.disk += cur_dir_size.disk;
    return sizes;
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

void append_entry(EntryList *list, Entry e)
{
    size_t new_size;
    if (list->count == list->capacity) {
        new_size = list->count == 0 ?  16 : list->capacity * 2;
        Entry *tmp; 
        tmp = realloc(list->items, new_size * sizeof *list->items);
        if (tmp == NULL) {
            perror("realloc");
            return;
        } 
        list->items = tmp;
        list->capacity = new_size;
    }
    list->items[list->count] = e;
    list->count++;
}

// collects the immediate children of path into an EntryList, with each directory child's sizes computed for its whole subtree.
EntryList collect_entries(const char *path)
{
    EntryList el = {.items=NULL, .count=0, .capacity=0};  
    
    DIR *dirp = opendir(path);
    if (dirp == NULL) {
        perror(path);
        return el;
    }
    struct dirent *dp;
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
        Entry e;
        Sizes tmp;
        if (S_ISDIR(st.st_mode)) {
            tmp = subtree_sizes(buf);
        } else {
            tmp.apparent = (long long)st.st_size;
            tmp.disk = (long long)st.st_blocks * 512;
        }
        if ((e.path = strdup(buf)) == NULL) {
            perror("strdup");
            continue;
        }
        e.apparent_size = tmp.apparent; 
        e.disk_usage = tmp.disk;
        append_entry(&el, e);
    }
    closedir(dirp);
    return el;
}

void print_entries(const EntryList *list)
{
    for (size_t i = 0; i < list->count; i++) {
        char apparent_buf[16];
        char disk_buf[16];
        format_size(apparent_buf, sizeof apparent_buf, list->items[i].apparent_size);
        format_size(disk_buf, sizeof disk_buf, list->items[i].disk_usage);
        printf("%s\t(%s apparent)\t%s\n", disk_buf, apparent_buf, list->items[i].path);
    }
}

// every path first, then items
void free_entries(EntryList *list)
{
    for (size_t i = 0; i < list->count; i++) {
        free(list->items[i].path);
    }
    free(list->items);
}


int compare_disk_desc(const void *e1, const void *e2) {
    const Entry *a = e1;
    const Entry *b = e2;
    
    if (a->disk_usage == b->disk_usage) {
        return strcmp(a->path, b->path);
    }
    return a->disk_usage > b->disk_usage ? -1 : 1; 
}

int compare_apparent_desc(const void *e1, const void *e2) {
    const Entry *a = e1;
    const Entry *b = e2;
    
    if (a->apparent_size == b->apparent_size) {
        return strcmp(a->path, b->path);
    }
    return a->apparent_size > b->apparent_size ? -1 : 1; 
}

void sort_entries(EntryList *list, int (*compare_func)(const void *, const void *)) {
    qsort(list->items, list->count, sizeof(Entry), compare_func);
}
