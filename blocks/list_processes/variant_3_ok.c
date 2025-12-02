#include <windows.h>  // Required for general Windows API functions and types
#include <tlhelp32.h> // Required for CreateToolhelp32Snapshot, Process32First/Next, PROCESSENTRY32
#include <stdio.h>    // Required for printf

/**
 * @brief Lists the executable names of all running processes.
 *
 * This function uses the Windows Tool Help Library API to take a snapshot
 * of all currently running processes and then iterates through them,
 * printing the executable name (szExeFile) of each process to the console.
 * It includes basic error handling for API calls.
 */
void ListProcesses() {
    HANDLE hProcessSnap = INVALID_HANDLE_VALUE; // Handle to the process snapshot
    PROCESSENTRY32 pe32;                        // Structure to hold process information

    // Take a snapshot of all currently running processes in the system.
    // TH32CS_SNAPPROCESS specifies that we want a snapshot of processes.
    // The second argument, 0, indicates all processes (not a specific PID).
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    // Check if the snapshot was created successfully.
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: CreateToolhelp32Snapshot failed. Error code: %lu\n", GetLastError());
        return; // Exit the function if snapshot creation failed
    }

    // Before using the PROCESSENTRY32 structure, its dwSize member must
    // be set to the size of the structure in bytes.
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process in the snapshot.
    // If successful, pe32 will contain data for the first process.
    if (!Process32First(hProcessSnap, &pe32)) {
        fprintf(stderr, "Error: Process32First failed. Error code: %lu\n", GetLastError());
        CloseHandle(hProcessSnap); // Clean up the snapshot handle
        return;                    // Exit the function
    }

    printf("--- Running Processes Executable Names ---\n");
    printf("------------------------------------------\n");

    // Iterate through all processes in the snapshot.
    // Process32Next retrieves information for the next process.
    // The loop continues as long as Process32Next successfully finds a process.
    do {
        // pe32.szExeFile contains the executable name of the current process.
        // It's a NULL-terminated string.
        printf("%s\n", pe32.szExeFile);
    } while (Process32Next(hProcessSnap, &pe32));

    printf("------------------------------------------\n");

    // Close the snapshot handle to free system resources.
    CloseHandle(hProcessSnap);
}

/*
// Example usage in a main function (uncomment to test):
int main() {
    ListProcesses();
    return 0;
}

// To compile this code using MinGW-w64 (GCC for Windows):
// gcc -o ListProcesses ListProcesses.c -lkernel32 -luser32
// (Actually, for this specific code, just `gcc -o ListProcesses ListProcesses.c` should be enough
// as `kernel32` is linked by default and `user32` is not needed here).
// `tlhelp32.h` functions are typically found in `kernel32.lib`.

// Example compilation and run:
// gcc ListProcesses.c -o ListProcesses
// .\ListProcesses.exe
*/