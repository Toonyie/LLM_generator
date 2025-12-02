#include <windows.h> // Required for Windows API functions
#include <tlhelp32.h> // Required for CreateToolhelp32Snapshot, PROCESSENTRY32, Process32First/Next
#include <stdio.h>    // Required for printf and fprintf

/**
 * @brief Lists the executable names of all running processes on the system.
 *
 * This function uses the ToolHelp32 API to take a snapshot of all running processes,
 * then iterates through the snapshot to print the executable name (szExeFile)
 * for each process. Error messages are printed to stderr if the snapshot
 * cannot be created or process information cannot be retrieved.
 */
void ListProcesses() {
    // Create a snapshot of all running processes.
    // TH32CS_SNAPPROCESS indicates we want a snapshot of processes.
    // The second argument (0) specifies all processes.
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    // Check if the snapshot creation was successful.
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: Could not create process snapshot. GetLastError: %lu\n", GetLastError());
        return;
    }

    // Declare a PROCESSENTRY32 structure to hold process information.
    PROCESSENTRY32 pe32;
    // Important: Set the size of the structure before calling Process32First.
    // This tells the API the version/size of the structure you are using.
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process in the snapshot.
    // If it fails, print an error and clean up.
    if (!Process32First(hSnapshot, &pe32)) {
        fprintf(stderr, "Error: Could not retrieve first process information. GetLastError: %lu\n", GetLastError());
        CloseHandle(hSnapshot); // Close the snapshot handle
        return;
    }

    // Print a header for clarity.
    printf("--- Running Processes ---\n");

    // Loop through all processes in the snapshot.
    // Process32Next returns TRUE if another process is found, FALSE otherwise.
    do {
        // pe32.szExeFile contains the base executable name (e.g., "notepad.exe").
        printf("%s\n", pe32.szExeFile);
    } while (Process32Next(hSnapshot, &pe32));

    // Close the snapshot handle to release system resources.
    CloseHandle(hSnapshot);
}

/*
// Example of how to compile and run this code:
//
// 1. Save the code as, for example, `process_lister.c`.
// 2. Compile using a C compiler (like MinGW GCC or Microsoft Visual C++):
//
//    For MinGW GCC:
//    gcc process_lister.c -o process_lister.exe -lkernel32
//
//    For Microsoft Visual C++ (Developer Command Prompt):
//    cl process_lister.c
//
// 3. Run the executable:
//    .\process_lister.exe
//
// Expected output will be a list of executable names, similar to:
//
// --- Running Processes ---
// System Idle Process
// System
// smss.exe
// csrss.exe
// wininit.exe
// services.exe
// lsass.exe
// ...
// explorer.exe
// chrome.exe
// Code.exe
// ...
*/

// Optional: A main function to demonstrate the ListProcesses function.
// Uncomment this section to make the file a complete runnable program.
/*
int main() {
    ListProcesses();
    return 0;
}
*/