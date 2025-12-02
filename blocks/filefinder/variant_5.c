#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

typedef struct {
    char** paths;
    size_t count;
    size_t capacity;
} FilePathList;

static void initialize_filepath_list(FilePathList* list, size_t initial_capacity) {
    list->paths = (char**)calloc(initial_capacity, sizeof(char*));
    list->count = 0;
    list->capacity = initial_capacity;
}

static void add_path_to_list(FilePathList* list, const char* path) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->paths = (char**)realloc(list->paths, list->capacity * sizeof(char*));
        if (list->paths == NULL) {
            perror("filefinder: Failed to reallocate path list");
            exit(EXIT_FAILURE);
        }
    }
    list->paths[list->count] = strdup(path);
    if (list->paths[list->count] == NULL) {
        perror("filefinder: Failed to duplicate path string");
        exit(EXIT_FAILURE);
    }
    list->count++;
}

void free_filepath_list(FilePathList* list) {
    for (size_t k = 0; k < list->count; ++k) {
        free(list->paths[k]);
    }
    free(list->paths);
    list->paths = NULL;
    list->count = 0;
    list->capacity = 0;
}

static void filefinder_recursive_search_variant5(const char* current_dir_path, const char* target_filename_part, FilePathList* found_files) {
    DIR* dir_handle;
    struct dirent* entry_pointer;
    struct stat stat_buffer;
    char full_entry_path[1024];

    if ((dir_handle = opendir(current_dir_path)) == NULL) {
        return;
    }

    while ((entry_pointer = readdir(dir_handle)) != NULL) {
        if (strcmp(entry_pointer->d_name, ".") == 0 || strcmp(entry_pointer->d_name, "..") == 0) {
            continue;
        }

        snprintf(full_entry_path, sizeof(full_entry_path), "%s/%s", current_dir_path, entry_pointer->d_name);

        if (stat(full_entry_path, &stat_buffer) == -1) {
            continue;
        }

        if (S_ISDIR(stat_buffer.st_mode)) {
            filefinder_recursive_search_variant5(full_entry_path, target_filename_part, found_files);
        } else if (S_ISREG(stat_buffer.st_mode)) {
            if (strstr(entry_pointer->d_name, target_filename_part) != NULL) {
                add_path_to_list(found_files, full_entry_path);
            }
        }
    }
    closedir(dir_handle);
}

FilePathList filefinder_search_variant5(const char* base_directory, const char* filename_substring) {
    FilePathList result_list;
    initialize_filepath_list(&result_list, 10);

    if (base_directory == NULL || filename_substring == NULL) {
        return result_list;
    }

    filefinder_recursive_search_variant5(base_directory, filename_substring, &result_list);

    return result_list;
}