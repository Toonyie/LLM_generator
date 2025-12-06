#on bash, type:
#pip install google-genai
#python generator.py


import os
from google import genai

#Replace API_KEY with another gemini key if you wish
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

#Function that types a prompt to the gemini llm
def llm_call(prompt: str) -> str:
    resp = client.models.generate_content(
        model="gemini-2.5-flash",   # or another Gemini model you have access to
        contents=prompt
    )
        
        
    return resp.text

#Test Prompt
print(llm_call("Say hello in one sentence."))
