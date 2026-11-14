import os
import random
import subprocess
from fastapi import FastAPI, HTTPException
from fastapi.responses import FileResponse
from pydantic import BaseModel
import db
from send_otp import send_mail

app = FastAPI()

CPP_ENGINE = "./ecotrade_engine.exe" if os.name == "nt" else "./ecotrade_engine"
otp_store = {}


def call_cpp(*args) -> list[str]:
    try:
        res = subprocess.run(
            [CPP_ENGINE, *[str(a) for a in args]],
            capture_output=True,
            text=True,
            timeout=5
        )
        output = res.stdout.strip()
        if not output:
            raise HTTPException(status_code=500, detail="C++ Engine returned empty response")
        parts = output.split("|")
        if parts[0] == "ERROR":
            raise HTTPException(status_code=400, detail=parts[1])
        return parts
    except HTTPException:
        raise
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"C++ Backend Error: {str(e)}")


class OtpRequest(BaseModel):
    email: str

class SignupRequest(BaseModel):
    student_id: str
    name: str
    email: str
    mobile: str
    blood_group: str = "O+"
    password: str
    otp: str

class LoginRequest(BaseModel):
    student_id: str
    password: str

class ItemRequest(BaseModel):
    owner_id: str
    title: str
    category: str = "General"
    condition: str = "Good"
    expected_return: str = "Open to offers"

class OfferRequest(BaseModel):
    item_id: int
    bidder_id: str
    offer_text: str

class AcceptOfferRequest(BaseModel):
    item_id: int
    offer_id: int
    owner_id: str

class BloodRequestModel(BaseModel):
    blood_group: str
    units: int
    urgency: int = 1


@app.get("/")
def serve_login():
    return FileResponse("login.html")

@app.get("/dashboard")
def serve_dashboard():
    return FileResponse("dashboard.html")

@app.get("/api/cpp-status")
def cpp_status():
    parts = call_cpp("STATUS")
    return {"status": parts[0], "engine": parts[1]}


@app.post("/api/auth/send-otp")
def send_otp(req: OtpRequest):
    otp = str(random.randint(100000, 999999))
    otp_store[req.email] = otp
    sent = send_mail(
        req.email,
        "EcoTrade Campus - Verification OTP",
        f"Your EcoTrade verification OTP is: {otp}\nValid for 10 minutes."
    )
    if not sent:
        raise HTTPException(status_code=500, detail="Failed to send OTP email")
    return {"message": "OTP sent successfully"}


@app.post("/api/auth/signup")
def signup(req: SignupRequest):
    real_otp = otp_store.get(req.email, "")
    call_cpp("SIGNUP", req.student_id, req.name, req.email, req.mobile, req.password, req.otp, real_otp)

    conn = db.get_connection()
    cur = conn.cursor()
    try:
        cur.execute(
            """
            INSERT INTO users (student_id, name, email, mobile, blood_group, password)
            VALUES (%s, %s, %s, %s, %s, %s)
            """,
            (req.student_id, req.name, req.email, req.mobile, req.blood_group, req.password)
        )
        conn.commit()
        otp_store.pop(req.email, None)
        return {"message": "Account created via C++ Backend", "student_id": req.student_id}
    except Exception:
        conn.rollback()
        raise HTTPException(status_code=400, detail="Student ID or Email already exists")
    finally:
        cur.close()
        conn.close()


@app.post("/api/auth/login")
def login(req: LoginRequest):
    call_cpp("LOGIN", req.student_id, req.password)

    conn = db.get_connection()
    cur = conn.cursor()
    try:
        cur.execute(
            "SELECT student_id, name, email, mobile, blood_group FROM users WHERE student_id=%s AND password=%s",
            (req.student_id, req.password)
        )
        user = cur.fetchone()
        if not user:
            raise HTTPException(status_code=401, detail="Invalid Student ID or Password")
        return {
            "student_id": user[0],
            "name": user[1],
            "email": user[2],
            "mobile": user[3],
            "blood_group": user[4]
        }
    finally:
        cur.close()
        conn.close()


@app.get("/api/items")
def get_items(viewer_id: str = ""):
    conn = db.get_connection()
    cur = conn.cursor()
    try:
        cur.execute("""
            SELECT i.id, i.owner_id, u.name, u.mobile, i.title, i.category,
                   i.condition, i.expected_return, i.status, i.accepted_bidder_id, i.handshake_code
            FROM items i
            JOIN users u ON i.owner_id = u.student_id
            ORDER BY i.id DESC
        """)
        rows = cur.fetchall()
        items = []
        for r in rows:
            item_id, owner_id, owner_name, owner_mobile, title, cat, cond, exp, status, acc_bidder, code = r
            privacy = call_cpp("PRIVACY", viewer_id, owner_id, acc_bidder or "", status)[0]
            items.append({
                "id": item_id,
                "owner_id": owner_id,
                "owner_name": owner_name,
                "owner_mobile": owner_mobile if privacy == "SHOW" else "Hidden until accepted",
                "title": title,
                "category": cat,
                "condition": cond,
                "expected_return": exp,
                "status": status,
                "accepted_bidder_id": acc_bidder,
                "handshake_code": code if privacy == "SHOW" else None
            })
        return items
    finally:
        cur.close()
        conn.close()


@app.post("/api/items")
def create_item(req: ItemRequest):
    conn = db.get_connection()
    cur = conn.cursor()
    try:
        cur.execute(
            """
            INSERT INTO items (owner_id, title, category, condition, expected_return, status)
            VALUES (%s, %s, %s, %s, %s, 'OPEN')
            RETURNING id
            """,
            (req.owner_id, req.title, req.category, req.condition, req.expected_return)
        )
        new_id = cur.fetchone()[0]
        conn.commit()
        return {"message": "Item listed", "id": new_id}
    finally:
        cur.close()
        conn.close()


@app.post("/api/offers")
def send_offer(req: OfferRequest):
    conn = db.get_connection()
    cur = conn.cursor()
    try:
        cur.execute("SELECT owner_id, status FROM items WHERE id=%s", (req.item_id,))
        item = cur.fetchone()
        if not item:
            raise HTTPException(status_code=404, detail="Item not found")

        call_cpp("OFFER", item[0], req.bidder_id, item[1])

        cur.execute(
            "INSERT INTO offers (item_id, bidder_id, offer_text, status) VALUES (%s, %s, %s, 'PENDING')",
            (req.item_id, req.bidder_id, req.offer_text)
        )
        conn.commit()
        return {"message": "Offer sent via C++ Barter Engine"}
    finally:
        cur.close()
        conn.close()


@app.post("/api/offers/accept")
def accept_offer(req: AcceptOfferRequest):
    conn = db.get_connection()
    cur = conn.cursor()
    try:
        cur.execute("""
            SELECT o.bidder_id, u_owner.email, u_owner.mobile, u_bidder.email, u_bidder.mobile, i.title
            FROM offers o
            JOIN items i ON o.item_id = i.id
            JOIN users u_owner ON i.owner_id = u_owner.student_id
            JOIN users u_bidder ON o.bidder_id = u_bidder.student_id
            WHERE o.id=%s AND i.id=%s AND i.owner_id=%s
        """, (req.offer_id, req.item_id, req.owner_id))
        row = cur.fetchone()
        if not row:
            raise HTTPException(status_code=404, detail="Offer not found")

        bidder_id, owner_email, owner_mobile, bidder_email, bidder_mobile, title = row
        _, handshake_code = call_cpp("ACCEPT", req.owner_id, bidder_id)

        cur.execute(
            "UPDATE items SET status='ACCEPTED', accepted_bidder_id=%s, handshake_code=%s WHERE id=%s",
            (bidder_id, handshake_code, req.item_id)
        )
        cur.execute("UPDATE offers SET status='ACCEPTED' WHERE id=%s", (req.offer_id,))
        conn.commit()

        send_mail(owner_email, f"EcoTrade Deal Accepted: {title}",
                  f"Handshake Code: {handshake_code}\nBidder Mobile: {bidder_mobile}")
        send_mail(bidder_email, f"EcoTrade Offer Accepted: {title}",
                  f"Handshake Code: {handshake_code}\nOwner Mobile: {owner_mobile}")

        return {
            "message": "Contact Unlocked via C++ Barter Engine",
            "handshake_code": handshake_code,
            "owner_mobile": owner_mobile,
            "bidder_mobile": bidder_mobile
        }
    finally:
        cur.close()
        conn.close()


@app.post("/api/blood/dispatch")
def dispatch_blood(req: BloodRequestModel):
    conn = db.get_connection()
    cur = conn.cursor()
    try:
        cur.execute("SELECT units FROM blood_stock WHERE blood_group=%s", (req.blood_group,))
        row = cur.fetchone()
        if not row:
            raise HTTPException(status_code=404, detail="Blood group not found")

        available = row[0]
        _, remaining, priority_level = call_cpp("BLOOD", req.blood_group, available, req.units, req.urgency)

        cur.execute("UPDATE blood_stock SET units=%s WHERE blood_group=%s", (int(remaining), req.blood_group))
        conn.commit()
        return {
            "message": f"Dispatched via C++ Priority Queue ({priority_level})",
            "remaining_units": int(remaining)
        }
    finally:
        cur.close()
        conn.close()