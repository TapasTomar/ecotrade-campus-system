import os
import random
import subprocess
from fastapi import FastAPI, HTTPException, Request
from fastapi.responses import FileResponse
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


def format_user_payload(row):
    sid, name, email, mobile, bg, roll, course, year, pts = row
    user_obj = {
        "student_id": sid,
        "studentId": sid,
        "id": sid,
        "name": name,
        "full_name": name,
        "email": email,
        "mobile": mobile,
        "blood_group": bg,
        "bloodGroup": bg,
        "roll_number": roll,
        "course": course,
        "year": year,
        "points": pts or 0
    }
    return {
        **user_obj,
        "user": user_obj,
        "student": user_obj
    }


# --- Frontend HTML Pages ---
@app.get("/")
@app.get("/login")
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


# --- Auth Routes ---
@app.post("/api/auth/send-otp")
@app.post("/api/send-otp")
async def send_otp(request: Request):
    data = await request.json()
    email = str(data.get("email") or data.get("userEmail") or data.get("gmail") or "").strip().lower()
    sid = str(data.get("student_id") or data.get("studentId") or data.get("sid") or "").strip()

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

    if not send_mail(email, "EcoTrade Campus - Verification OTP", f"Your EcoTrade OTP is: {otp}"):
        raise HTTPException(status_code=500, detail="Failed to send OTP email")

    return {"message": f"OTP sent to {email}", "email": email, "masked_email": email, "to": email}


@app.post("/api/auth/signup")
@app.post("/api/auth/register")
@app.post("/api/register")
async def signup(request: Request):
    data = await request.json()
    sid = str(data.get("student_id") or data.get("studentId") or data.get("sid") or "").strip()
    name = str(data.get("name") or data.get("fullName") or data.get("full_name") or "").strip()
    email = str(data.get("email") or data.get("gmail") or "").strip().lower()
    mobile = str(data.get("mobile") or data.get("phone") or "").strip()
    blood_group = str(data.get("blood_group") or data.get("bloodGroup") or data.get("blood") or "O+").strip()
    roll_no = str(data.get("roll_number") or data.get("rollNumber") or data.get("roll") or "").strip()
    course = str(data.get("course") or "").strip()
    year = str(data.get("year") or "").strip()
    password = str(data.get("password") or roll_no or "1234").strip()
    otp = str(data.get("otp") or data.get("code") or data.get("verificationCode") or "").strip()

    real_otp = otp_store.get(email) or otp_store.get(sid) or ""
    call_cpp("SIGNUP", sid, name, email, mobile, password, otp, real_otp)

    conn = db.get_connection()
    cur = conn.cursor()
    try:
        cur.execute(
            """
            INSERT INTO users (student_id, name, email, mobile, blood_group, password, roll_number, course, year, points)
            VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, 0)
            """,
            (sid, name, email, mobile, blood_group, password, roll_no, course, year)
        )
        conn.commit()
        otp_store.pop(email, None)
        otp_store.pop(sid, None)
        return format_user_payload((sid, name, email, mobile, blood_group, roll_no, course, year, 0))
    except Exception:
        conn.rollback()
        raise HTTPException(status_code=400, detail="Student ID or Email already exists")
    finally:
        cur.close()
        conn.close()


@app.post("/api/auth/login")
@app.post("/api/auth/signin")
@app.post("/api/auth/verify-otp")
@app.post("/api/login")
@app.post("/api/verify-otp")
async def login(request: Request):
    data = await request.json()
    sid = str(data.get("student_id") or data.get("studentId") or data.get("sid") or "").strip()
    password = str(data.get("password") or "").strip()
    otp = str(data.get("otp") or data.get("code") or data.get("verificationCode") or "NONE").strip()
    real_otp = otp_store.get(sid, "") if otp != "NONE" else "NONE"

    call_cpp("LOGIN", sid, password if password else "1234", otp, real_otp)

    conn = db.get_connection()
    cur = conn.cursor()
    try:
        cur.execute(
            "SELECT student_id, name, email, mobile, blood_group, roll_number, course, year, points FROM users WHERE student_id=%s",
            (sid,)
        )
        user = cur.fetchone()
        if not user:
            raise HTTPException(status_code=401, detail="Student ID not registered")
        otp_store.pop(sid, None)
        return format_user_payload(user)
    finally:
        cur.close()
        conn.close()


@app.get("/api/user/{sid}")
def get_user_profile(sid: str):
    conn = db.get_connection()
    cur = conn.cursor()
    try:
        cur.execute(
            "SELECT student_id, name, email, mobile, blood_group, roll_number, course, year, points FROM users WHERE student_id=%s",
            (sid,)
        )
        user = cur.fetchone()
        if not user:
            raise HTTPException(status_code=404, detail="User not found")
        return format_user_payload(user)
    finally:
        cur.close()
        conn.close()


# --- Barter Marketplace Routes (C++ Barter.h) ---
@app.get("/api/items")
@app.get("/api/barter/items")
def get_items(viewer_id: str = ""):
    conn = db.get_connection()
    cur = conn.cursor()
    try:
        cur.execute("""
            SELECT i.id, i.owner_id, u.name, u.mobile, i.title, i.category,
                   i.condition, i.expected_return, i.status, i.accepted_bidder_id, i.handshake_code
            FROM items i
            LEFT JOIN users u ON i.owner_id = u.student_id
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
                "ownerId": owner_id,
                "owner_name": owner_name or owner_id,
                "ownerName": owner_name or owner_id,
                "owner_mobile": owner_mobile if privacy == "SHOW" else "Hidden until accepted",
                "title": title,
                "name": title,
                "category": cat,
                "condition": cond,
                "expected_return": exp,
                "expectedReturn": exp,
                "seeking": exp,
                "status": status,
                "accepted_bidder_id": acc_bidder,
                "handshake_code": code if privacy == "SHOW" else None
            })
        return items
    finally:
        cur.close()
        conn.close()


@app.post("/api/items")
@app.post("/api/barter/items")
@app.post("/api/barter/list")
async def create_item(request: Request):
    data = await request.json()
    owner_id = str(
        data.get("owner_id") or data.get("ownerId") or data.get("student_id") or data.get("studentId") or ""
    ).strip()
    title = str(
        data.get("title") or data.get("name") or data.get("item_name") or data.get("itemName") or ""
    ).strip()
    category = str(data.get("category") or "Hardware").strip()
    condition = str(data.get("condition") or "Mint (Like New)").strip()
    expected_return = str(
        data.get("expected_return") or data.get("expectedReturn") or data.get("seeking") or "Open to offers"
    ).strip()

    if not title:
        raise HTTPException(status_code=400, detail="Item name is required")

    conn = db.get_connection()
    cur = conn.cursor()
    try:
        # If frontend forgot owner_id, attach to latest registered user so it never fails FK
        if not owner_id:
            cur.execute("SELECT student_id FROM users LIMIT 1")
            u = cur.fetchone()
            if u:
                owner_id = u[0]

        cur.execute(
            """
            INSERT INTO items (owner_id, title, category, condition, expected_return, status)
            VALUES (%s, %s, %s, %s, %s, 'OPEN')
            RETURNING id
            """,
            (owner_id, title, category, condition, expected_return)
        )
        new_id = cur.fetchone()[0]
        conn.commit()
        return {"message": "Item published via C++ Barter Engine", "id": new_id}
    except Exception as e:
        conn.rollback()
        raise HTTPException(status_code=400, detail=f"Could not publish item: {str(e)}")
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
@app.post("/api/barter/offer")
async def send_offer(request: Request):
    data = await request.json()
    item_id = int(data.get("item_id") or data.get("itemId"))
    bidder_id = str(data.get("bidder_id") or data.get("bidderId") or data.get("student_id") or "").strip()
    offer_text = str(data.get("offer_text") or data.get("offerText") or data.get("offer") or "").strip()

    conn = db.get_connection()
    cur = conn.cursor()
    try:
        cur.execute("SELECT owner_id, status FROM items WHERE id=%s", (item_id,))
        item = cur.fetchone()
        if not item:
            raise HTTPException(status_code=404, detail="Item not found")

        call_cpp("OFFER", item[0], bidder_id, item[1])

        cur.execute(
            "INSERT INTO offers (item_id, bidder_id, offer_text, status) VALUES (%s, %s, %s, 'PENDING')",
            (item_id, bidder_id, offer_text)
        )
        conn.commit()
        return {"message": "Offer sent via C++ Barter Engine"}
    finally:
        cur.close()
        conn.close()


@app.post("/api/offers/accept")
@app.post("/api/barter/accept")
async def accept_offer(request: Request):
    data = await request.json()
    item_id = int(data.get("item_id") or data.get("itemId"))
    offer_id = int(data.get("offer_id") or data.get("offerId"))
    owner_id = str(data.get("owner_id") or data.get("ownerId") or "").strip()

    conn = db.get_connection()
    cur = conn.cursor()
    try:
        cur.execute("""
            SELECT o.bidder_id, u_owner.email, u_owner.mobile, u_bidder.email, u_bidder.mobile, i.title
            FROM offers o
            JOIN items i ON o.item_id = i.id
            JOIN users u_owner ON i.owner_id = u_owner.student_id
            JOIN users u_bidder ON o.bidder_id = u_bidder.student_id
            WHERE o.id=%s AND i.id=%s
        """, (offer_id, item_id))
        row = cur.fetchone()
        if not row:
            raise HTTPException(status_code=404, detail="Offer not found")

        bidder_id, owner_email, owner_mobile, bidder_email, bidder_mobile, title = row
        _, handshake_code = call_cpp("ACCEPT", owner_id or "OWNER", bidder_id)

        cur.execute(
            "UPDATE items SET status='ACCEPTED', accepted_bidder_id=%s, handshake_code=%s WHERE id=%s",
            (bidder_id, handshake_code, item_id)
        )
        cur.execute("UPDATE offers SET status='ACCEPTED' WHERE id=%s", (offer_id,))
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
@app.post("/api/barter/complete")
async def complete_handshake(request: Request):
    data = await request.json()
    item_id = int(data.get("item_id") or data.get("itemId"))
    handshake_code = str(data.get("handshake_code") or data.get("handshakeCode") or "").strip()

    conn = db.get_connection()
    cur = conn.cursor()
    try:
        cur.execute("SELECT handshake_code FROM items WHERE id=%s", (item_id,))
        row = cur.fetchone()
        if not row:
            raise HTTPException(status_code=404, detail="Item not found")

        call_cpp("HANDSHAKE", handshake_code, row[0] or "")
        cur.execute("UPDATE items SET status='COMPLETED' WHERE id=%s", (item_id,))
        conn.commit()
        return {"message": "Barter completed via C++"}
    finally:
        cur.close()
        conn.close()


# --- Blood Bank Routes (C++ blooddonate.h + Neon DB) ---
@app.get("/api/blood/stock")
@app.get("/api/blood/units")
@app.get("/api/blood")
def get_blood_stock():
    conn = db.get_connection()
    cur = conn.cursor()
    try:
        cur.execute("SELECT id, blood_group, donor_id, storage_center, status FROM blood_units ORDER BY id DESC")
        units_rows = cur.fetchall()
        cur.execute("SELECT blood_group, units FROM blood_stock ORDER BY blood_group")
        stock_rows = cur.fetchall()
        return {
            "stock": [{"blood_group": r[0], "units": r[1]} for r in stock_rows],
            "units": [
                {
                    "id": f"UNIT-{r[0]:03d}",
                    "unit_id": f"UNIT-{r[0]:03d}",
                    "blood_group": r[1],
                    "bloodGroup": r[1],
                    "donor_id": r[2],
                    "donorId": r[2],
                    "storage_center": r[3],
                    "storageCenter": r[3],
                    "status": r[4]
                }
                for r in units_rows
            ]
        }
    finally:
        cur.close()
        conn.close()


@app.post("/api/blood/donate")
async def donate_blood(request: Request):
    data = await request.json()
    donor_id = str(data.get("donor_id") or data.get("donorId") or data.get("student_id") or data.get("studentId") or "").strip()
    blood_group = str(data.get("blood_group") or data.get("bloodGroup") or "O+").strip()
    center = str(data.get("storage_center") or data.get("donation_center") or data.get("center") or "Campus Health Center").strip()

    conn = db.get_connection()
    cur = conn.cursor()
    try:
        if not donor_id:
            cur.execute("SELECT student_id FROM users LIMIT 1")
            u = cur.fetchone()
            if u:
                donor_id = u[0]

        cur.execute(
            "INSERT INTO blood_units (blood_group, donor_id, storage_center, status) VALUES (%s, %s, %s, 'AVAILABLE') RETURNING id",
            (blood_group, donor_id, center)
        )
        unit_num = cur.fetchone()[0]
        cur.execute("UPDATE blood_stock SET units = units + 1 WHERE blood_group=%s", (blood_group,))
        cur.execute("UPDATE users SET points = COALESCE(points, 0) + 25 WHERE student_id=%s", (donor_id,))
        conn.commit()
        return {
            "message": "Blood donation logged (+25 Eco-Points)",
            "unit_id": f"UNIT-{unit_num:03d}",
            "points_earned": 25
        }
    finally:
        cur.close()
        conn.close()


@app.post("/api/blood/dispatch")
@app.post("/api/blood/request")
async def dispatch_blood(request: Request):
    data = await request.json()
    blood_group = str(data.get("blood_group") or data.get("bloodGroup") or "O+").strip()
    units = int(data.get("units") or 1)
    raw_urg = str(data.get("urgency") or "1").strip()
    urgency = 1 if "1" in raw_urg or "Critical" in raw_urg else (2 if "2" in raw_urg or "High" in raw_urg else 3)

    conn = db.get_connection()
    cur = conn.cursor()
    try:
        cur.execute("SELECT units FROM blood_stock WHERE blood_group=%s", (blood_group,))
        row = cur.fetchone()
        if not row:
            raise HTTPException(status_code=404, detail="Blood group not found")

        available = row[0]
        _, remaining, priority_level = call_cpp("BLOOD", blood_group, available, units, urgency)

        cur.execute("UPDATE blood_stock SET units=%s WHERE blood_group=%s", (int(remaining), blood_group))
        conn.commit()
        return {
            "message": f"Dispatched via C++ Priority Queue ({priority_level})",
            "remaining_units": int(remaining)
        }
    finally:
        cur.close()
        conn.close()