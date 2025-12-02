#include <string.h>
#include <stdio.h> // Not strictly needed for this logic, but often useful for file operations

// Simple mock files for demonstration purposes
static const char* mock_files_v3[] = {
    "document.txt",
    "report.pdf",
    "image.jpg",
    "notes.md",
    "archive.zip",
    "data.csv",
    "config.ini",
    NULL // Sentinel value
};

/**
 * @brief Finds a file by name within a predefined mock list.
 *        This variant uses a switch statement based on the filename's first character
 *        to narrow down the search.
 *
 * @param filename The name of the file to find.
 * @return 1 if the file is found, 0 otherwise.
 */
int filefinder_find_v3(const char* filename) {
    if (filename == NULL || filename[0] == '\0') {
        return 0; // Invalid filename
    }

    char first_char = filename[0];

    switch (first_char) {
        case 'd':
            if (strcmp(filename, "document.txt") == 0) return 1;
            if (strcmp(filename, "data.csv") == 0) return 1;
            break;
        case 'r':
            if (strcmp(filename, "report.pdf") == 0) return 1;
            break;
        case 'i':
            if (strcmp(filename, "image.jpg") == 0) return 1;
            if (strcmp(filename, "config.ini") == 0) return 1; // "ini" starts with 'i'
            break;
        case 'n':
            if (strcmp(filename, "notes.md") == 0) return 1;
            break;
        case 'a':
            if (strcmp(filename, "archive.zip") == 0) return 1;
            break;
        default:
            // For any other first character, iterate through the full list as a fallback
            // or simply return 0 if no match found in specific cases.
            // For this variant, we'll iterate through the mock list.
            {
                int i = 0;
                while (mock_files_v3[i] != NULL) {
                    if (strcmp(filename, mock_files_v3[i]) == 0) {
                        return 1;
                    }
                    i++;
                }
            }
            break;
    }

    return 0; // File not found
}

// Example main function for testing (can be removed if only the block function is needed)
/*
int main() {
    printf("document.txt found: %d\n", filefinder_find_v3("document.txt")); // Expected: 1
    printf("report.pdf found: %d\n", filefinder_find_v3("report.pdf"));     // Expected: 1
    printf("missing.txt found: %d\n", filefinder_find_v3("missing.txt"));   // Expected: 0
    printf("image.jpg found: %d\n", filefinder_find_v3("image.jpg"));     // Expected: 1
    printf("data.csv found: %d\n", filefinder_find_v3("data.csv"));       // Expected: 1
    printf("config.ini found: %d\n", filefinder_find_v3("config.ini"));   // Expected: 1
    return 0;
}
*/