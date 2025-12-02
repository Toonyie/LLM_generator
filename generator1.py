import os
import random
import subprocess
import uuid
from google import genai

# ------------------------------------------
# CONFIGURATION
# ------------------------------------------

NUM_VARIANTS = 5
BLOCKS = ["keygen", "filefinder", "transformer1", "transformer2", "orchestrator"]

BLOCKS_DIR = "blocks"
OUTPUT_DIR = "generated_samples"

API_KEY = "AIzaSyBNm4UB7zHim_7Kp9bb62WdD8TKzj0VQQ8"
client = genai.Client(api_key=API_KEY)

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
def call_llm(prompt: str) -> str:
    resp = client.models.generate_content(
        model="gemini-2.5-flash",   # or another Gemini model you have access to
        contents=prompt
    )
    return resp.text

# ------------------------------------------
# 1. GENERATE VARIANTS
# ------------------------------------------

def generate_variants():
    os.makedirs(BLOCKS_DIR, exist_ok=True)

    for block in BLOCKS:
        block_path = os.path.join(BLOCKS_DIR, block)
        os.makedirs(block_path, exist_ok=True)

        for v in range(1, NUM_VARIANTS + 1):
            prompt = f"""
You are generating C code for block: {block}.
This is variant #{v}.

Follow the block description provided in class.

Rules:
1. Return ONLY valid raw C code.
2. Do NOT include any markdown formatting.
3. Do NOT include ``` or ```c or any code fences.
4. Do NOT include explanations, comments, or text outside the code.
5. The code must be syntactically different from previous variants.

Output must be C code ONLY.
"""
            code = call_llm(prompt)

            out_file = os.path.join(block_path, f"variant_{v}.c")
            with open(out_file, "w") as f:
                # f.write(code)
                f.write(clean_code(code))


            print(f"[+] Created {out_file}")


# ------------------------------------------
# 2. ASSEMBLE SAMPLE
# ------------------------------------------

def assemble_sample():
    sample_id = str(uuid.uuid4())[:8]
    sample_dir = os.path.join(OUTPUT_DIR, f"sample_{sample_id}")
    os.makedirs(sample_dir, exist_ok=True)

    combined_path = os.path.join(sample_dir, "combined.c")

    with open(combined_path, "w") as out:
        out.write("#include <stdio.h>\n#include <windows.h>\n\n")

        for block in BLOCKS:
            path = os.path.join(BLOCKS_DIR, block)
            variants = os.listdir(path)
            chosen = random.choice(variants)
            chosen_full = os.path.join(path, chosen)

            print(f"[+] Using {block}: {chosen}")

            with open(chosen_full, "r") as f:
                block_text = clean_code(f.read())
                out.write(block_text + "\n\n")

    return combined_path, sample_dir


# ------------------------------------------
# 3. "COMPILING"
# ------------------------------------------

def compile_to_pe(c_file, out_dir):
    exe_path = os.path.join(out_dir, "output.exe")

    # Replace with actual mingw path on your system
    cmd = ["x86_64-w64-mingw32-gcc", c_file, "-o", exe_path]

    print("[*] Compiling...")
    subprocess.run(cmd)
    print(f"[+] Output PE: {exe_path}")

    return exe_path


# ------------------------------------------
# MAIN
# ------------------------------------------

def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    generate_variants()
    c_file, folder = assemble_sample()
    compile_to_pe(c_file, folder)

    print("[✔] Done.")


if __name__ == "__main__":
    main()
