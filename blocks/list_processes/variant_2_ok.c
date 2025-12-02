#include <windows.h>  // For general Windows API functions and types
#include <tlhelp32.h> // For CreateToolhelp32Snapshot, Process32First/Next, PROCESSENTRY32
#include <stdio.h>    // For _ftprintf (which resolves to fprintf or fwprintf)
#include <tchar.h>    // For _tprintf and TEXT macro (for ANSI/Unicode compatibility)

/**
 * @brief Lists the executable names of all running processes on the system.
 *
 * This function uses the Windows Tool Help API to take a snapshot of all
 * active processes and then iterates through them, printing the executable
 * file name for each. It is designed to be compatible with both ANSI
 * and Unicode builds of a Windows application.
 */
void ListProcesses() {
    HANDLE hProcessSnap = INVALID_HANDLE_VALUE; // Handle to the process snapshot
    PROCESSENTRY32 pe32;                        // Structure to hold process information

    // Take a snapshot of all currently running processes.
    // TH32CS_SNAPPROCESS indicates we want processes.
    // The second parameter (0) is ignored for TH32CS_SNAPPROCESS.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    // Check if the snapshot creation failed.
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        _ftprintf(stderr, TEXT("Error: CreateToolhelp32Snapshot failed (Error Code: %lu).\n"), GetLastError());
        _ftprintf(stderr, TEXT("  This might be due to insufficient permissions. Try running as administrator.\n"));
        return;
    }

    // Before using the PROCESSENTRY32 structure, its dwSize member must be set
    // to the size of the structure. This is a common requirement for many
    // Windows API structures.
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process in the snapshot.
    // If successful, pe32 will be populated with data for the first process.
    if (!Process32First(hProcessSnap, &pe32)) {
        _ftprintf(stderr, TEXT("Error: Process32First failed (Error Code: %lu). No processes found or access denied.\n"), GetLastError());
        CloseHandle(hProcessSnap); // Ensure the snapshot handle is closed even on early failure.
        return;
    }

    _tprintf(TEXT("--- Running Processes (Executable Names) ---\n"));

    // Loop through the snapshot to get information for each process.
    // Process32Next retrieves the next process's information. It returns FALSE
    // when there are no more processes in the snapshot.
    do {
        // pe32.szExeFile contains the executable file name of the process.
        // The TEXT() macro and _tprintf ensure compatibility with ANSI/Unicode builds.
        _tprintf(TEXT("  %s\n"), pe32.szExeFile);
    } while (Process32Next(hProcessSnap, &pe32));

    _tprintf(TEXT("--------------------------------------------\n"));

    // Close the snapshot object handle to release system resources.
    CloseHandle(hProcessSnap);
}

/*
// Example of how to use the ListProcesses function in a main program:
#include <fcntl.h> // For _O_U8TEXT
#include <io.h>    // For _setmode

int main() {
    // Optional: For Visual Studio, to ensure wprintf output to console
    // correctly displays Unicode characters, especially if the console
    // itself is not set to a Unicode code page by default.
    // _setmode(_fileno(stdout), _O_U8TEXT);

    _tprintf(TEXT("Attempting to list processes...\n"));
    ListProcesses();
    _tprintf(TEXT("Finished listing processes.\n"));

    // Pause for user to see output if running as console application
    // system("pause"); // Not recommended for production, but useful for quick tests.
    return 0;
}
*/