```c
#include <windows.h> // Required for general Windows API functions and HANDLE
#include <tlhelp32.h> // Required for CreateToolhelp32Snapshot, PROCESSENTRY32, Process32First/Next
#include <stdio.h>   // Required for printf

/**
 * @brief Lists all running processes and prints their executable names.
 *
 * This function uses the Windows Tool Help Library (tlhelp32.h) to
 * take a snapshot of all processes currently running on the system.
 * It then iterates through this snapshot, printing the executable name
 * (szExeFile) for each process found.
 */
void ListProcesses() {
    HANDLE hProcessSnap = INVALID_HANDLE_VALUE; // Handle to the process snapshot
    PROCESSENTRY32 pe32;                      // Structure to hold process information

    // Take a snapshot of all processes in the system.
    // TH32CS_SNAPPROCESS indicates we want a snapshot of processes.
    // The second parameter (0) is ignored when TH32CS_SNAPPROCESS is used.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    // Check if the snapshot was created successfully.
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: CreateToolhelp32Snapshot failed. Error code: %lu\n", GetLastError());
        return; // Exit the function if snapshot creation failed
    }

    // Before using PROCESSENTRY32, its dwSize member must be set to the size of the structure.
    // This is crucial for the API functions to correctly fill the structure.
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process encountered in the snapshot.
    // If Process32First fails, it means there are no processes or an error occurred.
    if (!Process32First(hProcessSnap, &pe32)) {
        fprintf(stderr, "Error: Process32First failed. Error code: %lu\n", GetLastError());
        CloseHandle(hProcessSnap); // Always close the handle if it was opened
        return;
    }

    // Display information for each process, including the first one.
    // Loop through the snapshot using Process32Next until no more processes are found.
    printf("--- Running Processes ---\n");
    do {
        // Print the executable name of the current process.
        // szExeFile is a null-terminated string containing the executable name.
        printf("  %s\n", pe32.szExeFile);
    } while (Process32Next(hProcessSnap, &pe32)); // Continue to the next process

    printf("-------------------------\n");

    // Clean up: Close the handle to the process snapshot.
    // It's very important to close handles obtained from CreateToolhelp32Snapshot
    // to prevent resource leaks.
    CloseHandle(hProcessSnap);
}

// Optional: A main function to demonstrate how to call ListProcesses
int main() {
    ListProcesses();
    return 0;
}
```