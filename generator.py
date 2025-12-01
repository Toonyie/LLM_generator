import os
from google import genai

# The genai.Client() automatically looks for the GEMINI_API_KEY 
# or GOOGLE_API_KEY environment variable.
try:
    client = genai.Client()

    # Call the generate_content method with a model and your prompt
    response = client.models.generate_content(
        model="gemini-2.5-flash", 
        contents="Explain how AI works in a few words"
    )

    print(response.text)

except Exception as e:
    print(f"An error occurred: {e}")
    print("Please ensure your GEMINI_API_KEY environment variable is set correctly.")