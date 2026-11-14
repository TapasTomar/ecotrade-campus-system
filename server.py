import os
import random
import subprocess
from fastapi import FastAPI, HTTPException, Request
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
    otp: str = ""

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

class CompleteHandshakeRequest(BaseModel):
    item_id: int
    handshake_code: str

class BloodRequestModel(BaseModel):
    blood_group: str
    units: int
    urgency: int = 1


# --- Website Pages ---
@app.get("/")
@app.get("/login.html")
def serve_login():
    return FileResponse("login.html")

@app.get("/dashboard")
@app.get("/dashboard.html")
def serve_dashboard():
    return FileResponse("dashboard.html")

@app.get("/api/cpp-status")
def cpp_status():
    parts = call_cpp("STATUS")
    return {"status": parts[0], "engine": parts[1]}


# --- Auth API (C++ student.h + Neon DB) ---
@app.post("/api/auth/send-otp")
async def send_otp(request: Request):
    data = await request.json()
    email = str(data.get("email") or "").strip().lower()
    sid = str(data.get("student_id") or "").strip()

    if not email and sid:
        conn = db.get_connection()
        cur = conn.cursor()
        try:
            cur.execute("SELECT email FROM users WHERE student_id=%s", (sid,))
            row = cur.fetchone()
            if not row:
                raise HTTPException(status_code=404, detail="Student ID not registered")
            email = row[0].strip().lower()
        finally:
            cur.close()
            conn.close()

    if not email or "@" not in email:
        raise HTTPException(status_code=400, detail="Enter your Student ID or Email first")

    otp = str(random.randint(100000, 999999))
    otp_store[email] = otp
    if sid:
        otp_store[sid] = otp

    if not send_mail(email, "EcoTrade Campus - OTP", f"Your EcoTrade OTP is: {otp}"):
        raise HTTPException(status_code=500, detail="Failed to send OTP email")
    return {"message": f"OTP sent to {email}"}


@app.post("/api/auth/signup")
def signup(req: SignupRequest):
    email = req.email.strip().lower()
    sid = req.student_id.strip()
    real_otp = otp_store.get(email) or otp_store.get(sid) or ""

    call_cpp("SIGNUP", sid, req.name.strip(), email, req.mobile.strip(), req.password, req.otp.strip(), real_otp)

    conn = db.get_connection()
    cur = conn.cursor()
    try:
        cur.execute(
            """
            INSERT INTO users (student_id, name, email, mobile, blood_group, password)
            VALUES (%s, %s, %s, %s, %s, %s)
            """,
            (sid, req.name.strip(), email, req.mobile.strip(), req.blood_group, req.password)
        )
        conn.commit()
        otp_store.pop(email, None)
        otp_store.pop(sid, None)
        return {"message": "Account created via C++ Backend", "student_id": sid}
    except Exception:
        conn.rollback()
        raise HTTPException(status_code=400, detail="Student ID or Email already exists")
    finally:
        cur.close()
        conn.close()


@app.post("/api/auth/login")
def login(req: LoginRequest):
    sid = req.student_id.strip()
    user_otp = req.otp.strip() if req.otp.strip() else "NONE"
    real_otp = otp_store.get(sid, "") if user_otp != "NONE" else "NONE"

    call_cpp("LOGIN", sid, req.password, user_otp, real_otp)

    conn = db.get_connection()
    cur = conn.cursor()
    try:
        cur.execute(
            "SELECT student_id, name, email, mobile, blood_group FROM users WHERE student_id=%s AND password=%s",
            (sid, req.password)
        )
        user = cur.fetchone()
        if not user:
            raise HTTPException(status_code=401, detail="Invalid Student ID or Password")
        otp_store.pop(sid, None)
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


# --- Barter API (C++ Barter.h + Neon DB) ---
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


@app.get("/api/offers")
def get_offers(owner_id: str = ""):
    conn = db.get_connection()
    cur = conn.cursor()
    try:
        cur.execute("""
            SELECT o.id, o.item_id, i.title, o.bidder_id, u.name, u.mobile, o.offer_text, o.status
            FROM offers o
            JOIN items i ON o.item_id = i.id
            JOIN users u ON o.bidder_id = u.student_id
            WHERE i.owner_id=%s OR %s=''
            ORDER BY o.id DESC
        """, (owner_id, owner_id))
        rows = cur.fetchall()
        return [
            {
                "id": r[0],
                "item_id": r[1],
                "item_title": r[2],
                "bidder_id": r[3],
                "bidder_name": r[4],
                "bidder_mobile": r[5] if r[7] == "ACCEPTED" else "Hidden until accepted",
                "offer_text": r[6],
                "status": r[7]
            }
            for r in rows
        ]
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


@app.post("/api/offers/complete")
def complete_handshake(req: CompleteHandshakeRequest):
    conn = db.get_connection()
    cur = conn.cursor()
    try:
        cur.execute("SELECT handshake_code FROM items WHERE id=%s", (req.item_id,))
        row = cur.fetchone()
        if not row:
            raise HTTPException(status_code=404, detail="Item not found")

        call_cpp("HANDSHAKE", req.handshake_code.strip(), row[0] or "")
        cur.execute("UPDATE items SET status='COMPLETED' WHERE id=%s", (req.item_id,))
        conn.commit()
        return {"message": "Barter completed via C++"}
    finally:
        cur.close()
        conn.close()


# --- Blood Bank API (C++ blooddonate.h + Neon DB) ---
@app.get("/api/blood/stock")
def get_blood_stock():
    conn = db.get_connection()
    cur = conn.cursor()
    try:
        cur.execute("SELECT blood_group, units FROM blood_stock ORDER BY blood_group")
        return [{"blood_group": r[0], "units": r[1]} for r in cur.fetchall()]
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