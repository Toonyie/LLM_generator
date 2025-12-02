#include <stdlib.h>
#include <time.h>

// Define a structure to hold the generated key pair components.
// This allows returning multiple related values from the function.
typedef struct {
    unsigned long componentA; // Could represent a public modulus or a part of it
    unsigned long componentB; // Could represent a private exponent or a part of it
    unsigned int  checksum;   // A simple checksum for illustrative purposes
} KeyPair;

// Function to generate a key pair.
// This variant returns a struct directly, distinguishing it from functions
// that might take pointers to fill key components.
// It also takes a configuration parameter 'configFlags' to influence generation.
KeyPair keygen(unsigned int configFlags) {
    // Static flag to ensure srand() is called only once per program execution,
    // which is good practice for rand() based generators.
    static int rand_seeded = 0;
    if (!rand_seeded) {
        srand((unsigned int)time(NULL));
        rand_seeded = 1;
    }

    KeyPair newKey;

    // Generate two "large" pseudo-random numbers using multiple rand() calls
    // to potentially increase the range beyond what a single rand() might provide.
    unsigned long tempVal1 = ((unsigned long)rand() << 16) | rand();
    unsigned long tempVal2 = ((unsigned long)rand() << 16) | rand();

    // Ensure tempVal1 and tempVal2 are non-zero and distinct for operations below.
    // In a real crypto system, these would be securely generated primes.
    if (tempVal1 == 0) tempVal1 = 1;
    if (tempVal2 == 0) tempVal2 = 2;
    if (tempVal1 == tempVal2) tempVal2 += (tempVal1 % 100) + 1; // Make them different

    // Derive componentA (e.g., a public modulus 'n' in RSA-like systems)
    // using multiplication, which is syntactically different from simple arithmetic.
    newKey.componentA = tempVal1 * tempVal2;

    // Derive componentB (e.g., a private exponent 'd')
    // using a combination of addition and bit shift, illustrating different operations.
    newKey.componentB = (tempVal1 + tempVal2) >> 2;

    // Apply a simple modification based on configFlags,
    // to demonstrate conditional logic in key generation.
    if (configFlags & 0x01) { // If the first bit of configFlags is set
        newKey.componentA ^= 0xDEADBEEFBADF00DULL; // XOR with a constant
    }
    if (configFlags & 0x02) { // If the second bit is set
        newKey.componentB += tempVal1; // Add one of the temporary values
    }

    // Calculate a simple checksum based on the generated components.
    // This adds another data member to the struct.
    newKey.checksum = (unsigned int)(newKey.componentA % 65536) ^ (unsigned int)(newKey.componentB % 65536);

    return newKey;
}