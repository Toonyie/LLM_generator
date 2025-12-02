import sys, time
from google import genai

API_KEY = "AIzaSyBNm4UB7zHim_7Kp9bb62WdD8TKzj0VQQ8"
client = genai.Client(api_key=API_KEY)

def llm_call(prompt: str) -> str:
    resp = client.models.generate_content(
        model="gemini-2.5-flash",
        contents=prompt
    )
    return resp.text or ""

def clean(s: str) -> str:
    return s.strip().lower()

inp = sys.argv[1]
out_path = "results.txt"

yes = 0
no = 0
total = 0

with open(out_path, "w") as f:
    for raw in open(inp, "r", encoding="utf-8"):
        raw = raw.strip()
        if not raw:
            continue
        parts = raw.split()
        if len(parts) < 2:
            continue

        lib, func = parts[0], parts[1]
        total += 1

        prompt = (
            "What's the filename of the Windows DLL that exports the following function?\n"
            f"Function: {func}\n"
            "Answer with ONLY the DLL filename (example: kernel32.dll)."
        )

        ans = llm_call(prompt)
        ans_c = clean(ans)
        lib_c = clean(lib)

        if lib_c in ans_c:
            yes += 1
            line = f"YES = {lib} | {func} | ans={ans.strip()}"
        else:
            no += 1
            line = f"NO  = {lib} | {func} | ans={ans.strip()}"

        print(line)
        f.write(line + "\n")
        f.flush()

        time.sleep(1.0)  # small pause to avoid rate limits

print("\nSUMMARY")
print("Total:", total)
print("YES:", yes)
print("NO :", no)
print("YES rate:", yes/total if total else 0)
