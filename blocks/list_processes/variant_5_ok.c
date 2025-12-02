#include <windows.h> // Required for general Windows API functions and types (e.g., HANDLE, DWORD)
#include <tlhelp32.h> // Required for process enumeration functions (e.g., CreateToolhelp32Snapshot, PROCESSENTRY32)
#include <stdio.h>   // Required for printf and fprintf

/**
 * @brief Lists all running processes and prints their executable names.
 *
 * This function uses the Windows Tool Help Library (tlhelp32.h) to
 * take a snapshot of all active processes and then iterates through them,
 * printing the name of each executable file.
 *
 * Error messages are printed to stderr if any API call fails.
 */
void ListProcesses() {
    HANDLE hProcessSnap = INVALID_HANDLE_VALUE; // Handle to the process snapshot
    PROCESSENTRY32 pe32;                      // Structure to hold process information

    // Take a snapshot of all processes in the system.
    // TH32CS_SNAPPROCESS includes all processes.
    // The second argument (process ID) is ignored for TH32CS_SNAPPROCESS.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    // Check if the snapshot was created successfully.
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: CreateToolhelp32Snapshot failed. GetLastError: %lu\n", GetLastError());
        return; // Exit the function if we can't get a snapshot.
    }

    // Set the size of the structure before using it.
    // This is crucial for the API functions to work correctly.
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process.
    // If it fails, there might be no processes or an internal error.
    if (!Process32First(hProcessSnap, &pe32)) {
        fprintf(stderr, "Error: Process32First failed. GetLastError: %lu\n", GetLastError());
        CloseHandle(hProcessSnap); // Always close the handle!
        return;
    }

    printf("--- Running Processes Executable Names ---\n");

    // Loop through all processes until Process32Next returns FALSE.
    do {
        // Print the executable file name.
        // pe32.szExeFile is a TCHAR array (usually char or wchar_t depending on UNICODE define).
        // Since we are using standard printf and not wprintf, we assume MBCS/ANSI for simplicity.
        // For full UNICODE compatibility, one would typically use _tprintf and LPTSTR.
        printf("%s\n", pe32.szExeFile);
    } while (Process32Next(hProcessSnap, &pe32)); // Get the next process information

    printf("------------------------------------------\n");

    // Clean up: Close the snapshot handle.
    // It's important to release system resources.
    CloseHandle(hProcessSnap);
}

/*
// Example of how to use the ListProcesses function in a main program:
int main() {
    ListProcesses();
    
    printf("\nPress Enter to exit...");
    getchar(); // Keep console open until user presses enter
    
    return 0;
}
*/