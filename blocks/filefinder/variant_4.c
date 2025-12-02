#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define FINDER_PATH_BUFFER_SIZE 1024

void find_target_file(const char* current_dir_path, const char* target_file_name) {
    DIR* p_dir = NULL;
    struct dirent* p_entry = NULL;
    struct stat entry_stat_info;
    char path_buffer[FINDER_PATH_BUFFER_SIZE];

    p_dir = opendir(current_dir_path);
    if (NULL == p_dir) {
        fprintf(stderr, "filefinder: Cannot open directory '%s'\n", current_dir_path);
        return;
    }

    do {
        p_entry = readdir(p_dir);

        if (NULL == p_entry) {
            break;
        }

        if (!strcmp(p_entry->d_name, ".") || !strcmp(p_entry->d_name, "..")) {
            continue;
        }

        sprintf(path_buffer, "%s/%s", current_dir_path, p_entry->d_name);

        if (-1 == stat(path_buffer, &entry_stat_info)) {
            fprintf(stderr, "filefinder: Failed to get stat for '%s'\n", path_buffer);
            continue;
        }

        if (S_ISDIR(entry_stat_info.st_mode)) {
            find_target_file(path_buffer, target_file_name);
        } else if (S_ISREG(entry_stat_info.st_mode)) {
            if (!strcmp(p_entry->d_name, target_file_name)) {
                printf("filefinder: Found '%s' at '%s'\n", target_file_name, path_buffer);
            }
        }
    } while (1);

    closedir(p_dir);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <start_directory> <file_to_find>\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("filefinder: Searching for '%s' in '%s'...\n", argv[2], argv[1]);
    find_target_file(argv[1], argv[2]);
    printf("filefinder: Search completed.\n");

    return EXIT_SUCCESS;
}