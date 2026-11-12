import os
import smtplib
from email.message import EmailMessage
from dotenv import load_dotenv
load_dotenv()

MY_EMAIL = os.getenv("MY_EMAIL", "")
MY_APP_PASSWORD = os.getenv("MY_APP_PASSWORD", "")

def send_mail(to_email: str, subject: str, body: str) -> bool:
    if not MY_EMAIL or not MY_APP_PASSWORD:
        print("Mail Error: MY_EMAIL or MY_APP_PASSWORD not set in environment.")
        return False
    try:
        msg = EmailMessage()
        msg["Subject"] = subject
        msg["From"] = MY_EMAIL
        msg["To"] = to_email
        msg.set_content(body)

        with smtplib.SMTP_SSL("smtp.gmail.com", 465) as smtp:
            smtp.login(MY_EMAIL, MY_APP_PASSWORD.replace(" ", ""))
            smtp.send_message(msg)
        return True
    except Exception as e:
        print("Mail Error:", e)
        return False