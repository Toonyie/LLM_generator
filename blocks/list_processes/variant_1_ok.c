#include <windows.h> // Required for general Windows API functions and data types
#include <tlhelp32.h>  // Required for CreateToolhelp32Snapshot, Process32First, Process32Next
#include <stdio.h>     // Required for printf

/**
 * @brief Lists all running processes and prints their executable names to the console.
 *
 * This function uses the Windows Tool Help Library (tlhelp32.h) to take a snapshot
 * of all running processes and then iterates through them, printing the executable
 * file name for each.
 */
void ListProcesses() {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    // Check if the snapshot was created successfully
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: Could not create process snapshot. GetLastError: %lu\n", GetLastError());
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32); // Must set dwSize before calling Process32First/Next

    // Get the first process in the snapshot
    if (!Process32First(hSnapshot, &pe32)) {
        fprintf(stderr, "Error: Could not retrieve first process. GetLastError: %lu\n", GetLastError());
        CloseHandle(hSnapshot); // Always close the handle
        return;
    }

    printf("--- Currently Running Processes ---\n");
    // Loop through all processes
    do {
        // Print the executable file name
        // pe32.szExeFile contains the name (e.g., "notepad.exe")
        printf("%s\n", pe32.szExeFile);
    } while (Process32Next(hSnapshot, &pe32)); // Move to the next process

    printf("-----------------------------------\n");

    // Close the snapshot handle to release system resources
    CloseHandle(hSnapshot);
}

/*
// Example of how to use the function (optional main for testing)
int main() {
    ListProcesses();
    
    printf("\nPress Enter to exit...");
    getchar(); // Wait for user input before closing console
    
    return 0;
}
*/