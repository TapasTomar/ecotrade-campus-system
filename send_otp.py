import os
import requests
from dotenv import load_dotenv

load_dotenv()

MY_EMAIL = os.getenv("MY_EMAIL", "").strip()
BREVO_API_KEY = os.getenv("BREVO_API_KEY", "").strip()

def send_mail(to_email: str, subject: str, body: str) -> bool:
    if not MY_EMAIL or not BREVO_API_KEY:
        print("Mail Error: MY_EMAIL or BREVO_API_KEY not set.")
        return False
    try:
        res = requests.post(
            "https://api.brevo.com/v3/smtp/email",
            headers={
                "accept": "application/json",
                "api-key": BREVO_API_KEY,
                "content-type": "application/json",
            },
            json={
                "sender": {"name": "EcoTrade Campus", "email": MY_EMAIL},
                "to": [{"email": to_email}],
                "subject": subject,
                "textContent": body,
            },
            timeout=10,
        )
        if res.status_code in (200, 201, 202):
            return True
        print("Brevo API Error:", res.status_code, res.text)
        return False
    except Exception as e:
        print("Mail Error:", e)
        return False