#on bash, type:
#pip install google-genai
#setx GEMINI_API_KEY "AIzaSyBNm4UB7zHim_7Kp9bb62WdD8TKzj0VQQ8"
#python generator.py


import os
from google import genai

API_KEY = "AIzaSyBNm4UB7zHim_7Kp9bb62WdD8TKzj0VQQ8"
client = genai.Client(api_key=API_KEY)

#Function that types a prompt to the gemini llm
def llm_call(prompt: str) -> str:
    resp = client.models.generate_content(
        model="gemini-2.5-flash",   # or another Gemini model you have access to
        contents=prompt
    )
    return resp.text

print(llm_call("Say hello in one sentence."))
