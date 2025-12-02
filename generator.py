#on bash, type:
#pip install google-genai
#python generator.py

from google import genai
from pathlib import Path

#Replace API_KEY with another gemini key if you wish
API_KEY = "AIzaSyBNm4UB7zHim_7Kp9bb62WdD8TKzj0VQQ8"
client = genai.Client(api_key=API_KEY)

#Function that types a prompt to the gemini llm
def llm_call(prompt: str) -> str:
    resp = client.models.generate_content(
        model="gemini-2.5-flash",   # or another Gemini model you have access to
        contents=prompt
    )
        
        
    return resp.text or ""

#Test Prompt
print(llm_call("Say hello in one sentence."))

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

out_dir = Path("blocks_out")
out_dir.mkdir(exist_ok=True)

for name, prompt in BLOCK_PROMPTS.items():
    print("="*80)
    print("Generating block:", name)
    print("="*80)
    code = llm_call(prompt)
    (out_dir / f"{name}.c").write_text(code, encoding="utf-8")
    print(code[:600])
    print(f"\nSaved to {out_dir/name}.c\n")
