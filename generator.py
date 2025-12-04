#on bash, type:
#pip install google-genai
#python generator.py
import os
import subprocess
import tempfile
import time
import random
import subprocess
import uuid

from google import genai
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


#Replace API_KEY with another gemini key if you wish
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
    """
    Removes markdown fences like ``` and ```c from LLM output.
    Returns raw C code only.
    """
    if not text:
        return ""

    # Remove backticks
    cleaned = text.replace("```c", "").replace("```C", "").replace("```", "")

    # Strip whitespace
    return cleaned.strip()

#Function that types a prompt to the gemini llm
def llm_call(prompt: str) -> str:
    resp = client.models.generate_content(
        model="gemini-2.5-flash",   # or another Gemini model you have access to
        contents=prompt
    )   
    return resp.text or ""

BLOCK_PROMPTS = {
    "cpu_vendor": r"""
// Code in C
// Code for Windows
// Use the Windows API or intrinsics
// Write a function
// Function prototype is: int GetCPUVendor(char *out, int outLen);
// Fill out with the CPU vendor string (like GenuineIntel or AuthenticAMD).
// Return 0 on success, -1 on failure.
""",
    "list_processes": r"""
// Code in C
// Code for Windows
// Use the Windows API
// Write a function
// Function prototype is: void ListProcesses();
// Print the executable name of each running process.
""",
    "read_file": r"""
// Code in C
// Code for Windows
// Use the Windows API
// Write a function
// Function prototype is: int ReadFileToBuffer(const char *path, void *buffer, int maxLen);
// Read up to maxLen bytes from path into buffer.
// Return number of bytes read, or -1 on error.
""",
    "base64_decode": r"""
// Code in C
// Code for Windows
// Use Windows APIs (Crypt32 or ATL is OK)
// Write a function
// Function prototype is: int Base64DecodeBuffer(const char *src, int srcLen, unsigned char *dst, int dstLen);
// Decode base64 from src into dst. Return number of decoded bytes, or -1 on error.
"""
}

#Only generate variants for blocks we can test/compile
BLOCKS = list(BLOCK_PROMPTS.keys())

#Tiny main() programs ONLY used to check compilation (see if it runs)
HARNESS = {
    "cpu_vendor": r"""
#include <windows.h>
#include <stdio.h>
#include <string.h>
int GetCPUVendor(char *out, int outLen);
int main() {
    char buf[64] = {0};
    int r = GetCPUVendor(buf, 64);
    printf("r=%d vendor=%s\n", r, buf);
    return 0;
}
""",
    "list_processes": r"""
#include <windows.h>
#include <stdio.h>
void ListProcesses();
int main() {
    ListProcesses();
    return 0;
}
""",
    "read_file": r"""
#include <windows.h>
#include <stdio.h>
int ReadFileToBuffer(const char *path, void *buffer, int maxLen);
int main() {
    char buf[64] = {0};
    int n = ReadFileToBuffer("C:\\Windows\\win.ini", buf, 63);
    printf("n=%d buf=%s\n", n, buf);
    return 0;
}
""",
    "base64_decode": r"""
#include <windows.h>
#include <stdio.h>
#include <string.h>
int Base64DecodeBuffer(const char *src, int srcLen, unsigned char *dst, int dstLen);
int main() {
    const char *b64 = "SGVsbG8="; // "Hello"
    unsigned char out[64] = {0};
    int n = Base64DecodeBuffer(b64, (int)strlen(b64), out, 64);
    printf("n=%d out=%s\n", n, out);
    return 0;
}
"""
}

COMBINED_HARNESS = r"""
int main() {
    char vendor[64] = {0};
    GetCPUVendor(vendor, 64);
    printf("CPU vendor: %s\n", vendor);

    printf("Processes:\n");
    ListProcesses();

    char buf[128] = {0};
    int n = ReadFileToBuffer("C:\\\\Windows\\\\win.ini", buf, 127);
    printf("ReadFileToBuffer n=%d text=%s\n", n, buf);

    const char *b64 = "SGVsbG8=";
    unsigned char out[64] = {0};
    int m = Base64DecodeBuffer(b64, (int)strlen(b64), out, 64);
    printf("Base64DecodeBuffer m=%d out=%s\n", m, out);

    return 0;
}
"""

# --------- Compilation Check ---------------
#Helper function that checks if the code compiles
def try_compile(block_name: str, code: str):
    """
    Try to compile the LLM-generated block by adding a tiny harness.
    Returns (success_bool, compiler_output_string).
    This assumes you have gcc (MinGW-w64) on PATH.
    """
    harness = HARNESS[block_name]

    #Makes a temporary c file and executable file to test compilation
    with tempfile.TemporaryDirectory() as td:
        cpath = Path(td) / f"{block_name}.c"
        exepath = Path(td) / f"{block_name}.exe"

        #Combine block + harness (main function) so compiler can test it
        full_source = PRELUDE + "\n" + code + "\n\n" + harness
        cpath.write_text(full_source, encoding="utf-8")

        #Compile attempt (link common Windows libs just in case)
        cmd = [
            "gcc",
            str(cpath),
            "-o", str(exepath),
            "-lcrypt32",
            "-lpsapi"
        ]
        proc = subprocess.run(cmd, capture_output=True, text=True)
        compiler_log = (proc.stdout or "") + (proc.stderr or "")
        return proc.returncode == 0, compiler_log
    
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
                code = clean_code(raw)          # 2) strip fences / junk
                ok, log = try_compile(block, code)  # 3) compile cleaned code
                if ok:
                    out_file = os.path.join(block_path, f"variant_{v}_ok.c")
                    with open(out_file, "w", encoding="utf-8") as f:
                        f.write(clean_code(code))
                    print(f"✅ Compiled. Saved {out_file}")
                    break
                else:
                    print("❌ Compile failed. Log head:")
                    print(log[:800])
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

    generate_variants()
    c_file, folder = assemble_sample()
    compile_to_pe(c_file, folder)

    print("[✔] Done.")

if __name__ == "__main__":
    main()

