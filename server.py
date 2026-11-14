from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import FileResponse
from pydantic import BaseModel
import os
import time
import random
from send_otp import send_mail
from db import get_conn, init_db

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

BASE_DIR = os.path.dirname(os.path.abspath(__file__))

otp_store = {}
otp_attempts = {}

@app.on_event("startup")
def startup_event():
    init_db()

# ---------------- DB HELPERS ----------------

def find_student(student_id: int):
    conn = get_conn()
    cur = conn.cursor()
    cur.execute("SELECT * FROM students WHERE student_id = %s", (student_id,))
    row = cur.fetchone()
    cur.close()
    conn.close()
    if not row:
        return None
    return {
        "studentId": row["student_id"],
        "name": row["name"],
        "rollNo": row["roll_no"],
        "course": row["course"],
        "year": row["year"],
        "email": row["email"],
        "phone": row["phone"],
        "bloodGroup": row["blood_group"],
        "ecoPoints": row["eco_points"]
    }

def add_eco_points(student_id: int, points: int):
    conn = get_conn()
    cur = conn.cursor()
    cur.execute("UPDATE students SET eco_points = eco_points + %s WHERE student_id = %s", (points, student_id))
    conn.commit()
    cur.close()
    conn.close()

# ---------------- REQUEST MODELS ----------------

class SendOtpReq(BaseModel):
    studentId: int
    email: str = ""

class VerifyLoginReq(BaseModel):
    studentId: int
    otp: int

class RegisterReq(BaseModel):
    studentId: int
    name: str
    rollNo: str
    course: str
    year: int
    email: str
    phone: str
    bloodGroup: str
    otp: int

class AddItemReq(BaseModel):
    ownerId: int
    name: str
    condition: str
    wantItem: str

class OfferReq(BaseModel):
    buyerId: int
    offerItem: str

class OwnerReq(BaseModel):
    ownerId: int

class CancelOfferReq(BaseModel):
    buyerId: int

class HandshakeReq(BaseModel):
    otp: int

class DonateBloodReq(BaseModel):
    donorId: int
    bloodGroup: str
    centerName: str

class DispatchBloodReq(BaseModel):
    hospitalName: str
    requiredGroup: str
    urgency: int

# ---------------- HTML PAGES SERVE ----------------

@app.get("/")
def serve_root():
    return FileResponse(os.path.join(BASE_DIR, "login.html"))

@app.get("/login.html")
def serve_login_page():
    return FileResponse(os.path.join(BASE_DIR, "login.html"))

@app.get("/dashboard.html")
def serve_dashboard_page():
    return FileResponse(os.path.join(BASE_DIR, "dashboard.html"))

# ---------------- AUTH APIs ----------------

@app.get("/api/students/{student_id}")
def get_student_profile(student_id: int):
    s = find_student(student_id)
    if not s:
        raise HTTPException(status_code=404, detail="Student not found")
    return s

@app.post("/api/auth/send-otp")
def send_otp(req: SendOtpReq):
    if req.studentId < 10000000000 or req.studentId > 99999999999:
        raise HTTPException(status_code=400, detail="Student ID must be 11 digits!")

    found_user = find_student(req.studentId)
    target_email = req.email.strip()
    if target_email == "" and found_user is not None:
        target_email = found_user["email"]

    if target_email == "":
        raise HTTPException(status_code=404, detail="Student ID not registered. Please Create Account first.")

    otp = random.randint(100000, 999999)
    otp_store[req.studentId] = otp
    otp_attempts[req.studentId] = 3

    msg = f"Hello,\n\nYour EcoTrade 6-Digit Verification OTP is: {otp}"
    sent = send_mail(target_email, f"EcoTrade Verification OTP: {otp}", msg)

    if not sent:
        raise HTTPException(status_code=500, detail="Failed to send email. Verify MY_EMAIL & MY_APP_PASSWORD.")

    return {"message": f"OTP sent to {target_email}", "email": target_email}

@app.post("/api/auth/verify-login")
def verify_login(req: VerifyLoginReq):
    if req.studentId not in otp_store:
        raise HTTPException(status_code=400, detail="Please click Send OTP first!")

    if otp_attempts[req.studentId] <= 0:
        raise HTTPException(status_code=403, detail="Security Lock: 3 wrong attempts!")

    if otp_store[req.studentId] != req.otp:
        otp_attempts[req.studentId] -= 1
        left = otp_attempts[req.studentId]
        raise HTTPException(status_code=400, detail=f"Wrong OTP! Attempts left: {left}")

    user = find_student(req.studentId)
    if not user:
        raise HTTPException(status_code=404, detail="Student not registered!")

    del otp_store[req.studentId]
    return {"verified": True, "student": user}

@app.post("/api/auth/register")
def register_student(req: RegisterReq):
    if req.studentId not in otp_store or otp_store[req.studentId] != req.otp:
        raise HTTPException(status_code=400, detail="Invalid or unverified 6-digit OTP!")

    if find_student(req.studentId):
        raise HTTPException(status_code=400, detail="Student ID already registered!")

    conn = get_conn()
    cur = conn.cursor()
    cur.execute("""
        INSERT INTO students (student_id, name, roll_no, course, year, email, phone, blood_group, eco_points)
        VALUES (%s, %s, %s, %s, %s, %s, %s, %s, 10)
    """, (req.studentId, req.name, req.rollNo, req.course, req.year, req.email, req.phone, req.bloodGroup))
    conn.commit()
    cur.close()
    conn.close()

    del otp_store[req.studentId]
    new_student = find_student(req.studentId)
    return {"verified": True, "student": new_student}

# ---------------- BARTER APIs ----------------

@app.get("/api/items")
def get_items(order: str = "forward", q: str = "", onlyAvailable: bool = False):
    conn = get_conn()
    cur = conn.cursor()
    dir_sql = "DESC" if order == "reverse" else "ASC"
    cur.execute(f"SELECT * FROM barter_items ORDER BY id {dir_sql}")
    rows = cur.fetchall()
    cur.close()
    conn.close()

    result = []
    for r in rows:
        if onlyAvailable and r["status"] != "AVAILABLE":
            continue
        if q != "":
            key = q.lower()
            if key not in r["name"].lower() and key not in r["want_item"].lower():
                continue

        owner = find_student(r["owner_id"])
        buyer = find_student(r["buyer_id"]) if r["buyer_id"] != 0 else None

        it = {
            "id": r["id"],
            "ownerId": r["owner_id"],
            "name": r["name"],
            "condition": r["condition"],
            "wantItem": r["want_item"],
            "status": r["status"],
            "buyerId": r["buyer_id"],
            "offerItem": r["offer_item"],
            "ownerName": owner["name"] if owner else "Student",
            "buyerName": buyer["name"] if buyer else "",
            "ownerPhone": (owner["phone"] if owner else "N/A") if r["status"] == "HANDSHAKE" else "",
            "buyerPhone": (buyer["phone"] if buyer else "N/A") if r["status"] == "HANDSHAKE" else ""
        }
        result.append(it)
    return result

@app.get("/api/items/history")
def get_trade_history():
    conn = get_conn()
    cur = conn.cursor()
    cur.execute("SELECT record FROM trade_history ORDER BY id DESC")
    rows = cur.fetchall()
    cur.close()
    conn.close()
    return [r["record"] for r in rows]

@app.post("/api/items")
def add_item(req: AddItemReq):
    conn = get_conn()
    cur = conn.cursor()
    cur.execute("""
        INSERT INTO barter_items (owner_id, name, condition, want_item, status, buyer_id, offer_item, otp_code)
        VALUES (%s, %s, %s, %s, 'AVAILABLE', 0, 'None', 0)
        RETURNING id
    """, (req.ownerId, req.name, req.condition, req.wantItem))
    new_id = cur.fetchone()["id"]
    conn.commit()
    cur.close()
    conn.close()
    return {"id": new_id, "name": req.name}

@app.post("/api/items/{item_id}/offer")
def send_offer(item_id: int, req: OfferReq):
    conn = get_conn()
    cur = conn.cursor()
    cur.execute("SELECT * FROM barter_items WHERE id = %s", (item_id,))
    it = cur.fetchone()
    if not it:
        cur.close(); conn.close()
        raise HTTPException(status_code=404, detail="Item not found")
    if it["owner_id"] == req.buyerId:
        cur.close(); conn.close()
        raise HTTPException(status_code=400, detail="Cannot swap your own item!")
    if it["status"] != "AVAILABLE":
        cur.close(); conn.close()
        raise HTTPException(status_code=400, detail="Item already has a pending offer!")

    cur.execute("""
        UPDATE barter_items SET status = 'REQUESTED', buyer_id = %s, offer_item = %s WHERE id = %s
    """, (req.buyerId, req.offerItem, item_id))
    conn.commit()
    cur.close()
    conn.close()
    return {"updated": True}

@app.post("/api/items/{item_id}/cancel")
def cancel_offer(item_id: int, req: CancelOfferReq):
    conn = get_conn()
    cur = conn.cursor()
    cur.execute("SELECT * FROM barter_items WHERE id = %s", (item_id,))
    it = cur.fetchone()
    if not it or it["status"] != "REQUESTED" or it["buyer_id"] != req.buyerId:
        cur.close(); conn.close()
        raise HTTPException(status_code=400, detail="No active offer from you!")

    cur.execute("""
        UPDATE barter_items SET status = 'AVAILABLE', buyer_id = 0, offer_item = 'None' WHERE id = %s
    """, (item_id,))
    conn.commit()
    cur.close()
    conn.close()
    return {"cancelled": True}

@app.post("/api/items/{item_id}/accept")
def accept_offer(item_id: int, req: OwnerReq):
    conn = get_conn()
    cur = conn.cursor()
    cur.execute("SELECT * FROM barter_items WHERE id = %s", (item_id,))
    it = cur.fetchone()
    if not it:
        cur.close(); conn.close()
        raise HTTPException(status_code=404, detail="Item not found")
    if it["owner_id"] != req.ownerId:
        cur.close(); conn.close()
        raise HTTPException(status_code=403, detail="Only owner can accept!")
    if it["status"] != "REQUESTED":
        cur.close(); conn.close()
        raise HTTPException(status_code=400, detail="No pending request!")

    otp = random.randint(1000, 9999)
    cur.execute("UPDATE barter_items SET status = 'HANDSHAKE', otp_code = %s WHERE id = %s", (otp, item_id))
    conn.commit()
    cur.close()
    conn.close()

    owner = find_student(it["owner_id"])
    buyer = find_student(it["buyer_id"])
    if owner:
        msg = f"Swap Accepted for {it['name']}!\nBuyer: {buyer['name'] if buyer else ''} ({buyer['phone'] if buyer else ''})\n4-Digit Handshake Code: {otp}"
        send_mail(owner["email"], f"EcoTrade Handshake Code: {otp}", msg)
    if buyer:
        msg = f"Your offer for {it['name']} was Accepted!\nOwner: {owner['name'] if owner else ''} ({owner['phone'] if owner else ''})\n4-Digit Handshake Code: {otp}"
        send_mail(buyer["email"], f"EcoTrade Handshake Code: {otp}", msg)

    return {"accepted": True}

@app.post("/api/items/{item_id}/reject")
def reject_offer(item_id: int, req: OwnerReq):
    conn = get_conn()
    cur = conn.cursor()
    cur.execute("SELECT * FROM barter_items WHERE id = %s", (item_id,))
    it = cur.fetchone()
    if not it or it["owner_id"] != req.ownerId:
        cur.close(); conn.close()
        raise HTTPException(status_code=403, detail="Only owner can reject!")

    cur.execute("UPDATE barter_items SET status = 'AVAILABLE', buyer_id = 0, offer_item = 'None' WHERE id = %s", (item_id,))
    conn.commit()
    cur.close()
    conn.close()
    return {"rejected": True}

@app.delete("/api/items/{item_id}")
def delete_item(item_id: int, ownerId: int):
    conn = get_conn()
    cur = conn.cursor()
    cur.execute("DELETE FROM barter_items WHERE id = %s AND owner_id = %s", (item_id, ownerId))
    conn.commit()
    cur.close()
    conn.close()
    return {"deleted": True}

@app.post("/api/items/{item_id}/verify")
def verify_handshake(item_id: int, req: HandshakeReq):
    conn = get_conn()
    cur = conn.cursor()
    cur.execute("SELECT * FROM barter_items WHERE id = %s", (item_id,))
    it = cur.fetchone()
    if not it or it["status"] != "HANDSHAKE":
        cur.close(); conn.close()
        raise HTTPException(status_code=400, detail="Not waiting for handover!")
    if it["otp_code"] != req.otp:
        cur.close(); conn.close()
        raise HTTPException(status_code=400, detail="Wrong 4-Digit Handshake Code!")

    record = f"Item: {it['name']} | Owner: {it['owner_id']} | Swapped With: {it['offer_item']} | Buyer: {it['buyer_id']}"
    cur.execute("INSERT INTO trade_history (record) VALUES (%s)", (record,))
    cur.execute("DELETE FROM barter_items WHERE id = %s", (item_id,))
    conn.commit()
    cur.close()
    conn.close()

    add_eco_points(it["owner_id"], 10)
    add_eco_points(it["buyer_id"], 10)
    return {"message": "Handover Confirmed! +10 Eco-Points added."}

# ---------------- BLOOD BANK APIs ----------------

@app.get("/api/blood")
def get_blood():
    conn = get_conn()
    cur = conn.cursor()
    cur.execute("SELECT * FROM blood_packets ORDER BY packet_id ASC")
    rows = cur.fetchall()
    cur.close()
    conn.close()
    return [{
        "packetId": r["packet_id"],
        "donorId": r["donor_id"],
        "bloodGroup": r["blood_group"],
        "centerName": r["center_name"]
    } for r in rows]

@app.get("/api/blood/history")
def get_blood_history():
    conn = get_conn()
    cur = conn.cursor()
    cur.execute("SELECT record FROM blood_history ORDER BY id DESC")
    rows = cur.fetchall()
    cur.close()
    conn.close()
    return [r["record"] for r in rows]

@app.post("/api/blood/donate")
def donate_blood(req: DonateBloodReq):
    conn = get_conn()
    cur = conn.cursor()
    now = int(time.time())
    ninety_days = 90 * 24 * 60 * 60

    cur.execute("SELECT last_donation FROM donor_cooldowns WHERE donor_id = %s", (req.donorId,))
    cd = cur.fetchone()
    if cd:
        passed = now - cd["last_donation"]
        if passed < ninety_days:
            days_left = (ninety_days - passed) // (24 * 60 * 60)
            cur.close(); conn.close()
            raise HTTPException(status_code=400, detail=f"90-day recovery window active ({days_left} days remaining).")

    cur.execute("""
        INSERT INTO blood_packets (donor_id, blood_group, center_name)
        VALUES (%s, %s, %s) RETURNING packet_id
    """, (req.donorId, req.bloodGroup, req.centerName))
    pkt_id = cur.fetchone()["packet_id"]

    cur.execute("""
        INSERT INTO donor_cooldowns (donor_id, last_donation)
        VALUES (%s, %s)
        ON CONFLICT (donor_id) DO UPDATE SET last_donation = EXCLUDED.last_donation
    """, (req.donorId, now))

    conn.commit()
    cur.close()
    conn.close()

    add_eco_points(req.donorId, 25)
    return {"packetId": pkt_id}

@app.post("/api/blood/dispatch")
def dispatch_blood(req: DispatchBloodReq):
    conn = get_conn()
    cur = conn.cursor()
    tier = "Routine"
    if req.urgency == 3:
        tier = "Critical — ICU / Trauma"
    elif req.urgency == 2:
        tier = "High — Scheduled Surgery"

    cur.execute("SELECT * FROM blood_packets WHERE UPPER(blood_group) = UPPER(%s) ORDER BY packet_id ASC LIMIT 1", (req.requiredGroup,))
    pkt = cur.fetchone()
    match_type = "EXACT_MATCH"

    if not pkt:
        cur.execute("SELECT * FROM blood_packets WHERE UPPER(blood_group) = 'O-' ORDER BY packet_id ASC LIMIT 1")
        pkt = cur.fetchone()
        match_type = "UNIVERSAL_O_NEG_FALLBACK"

    if not pkt:
        cur.close(); conn.close()
        raise HTTPException(status_code=404, detail=f"Shortage: Neither {req.requiredGroup} nor O- available right now.")

    cur.execute("DELETE FROM blood_packets WHERE packet_id = %s", (pkt["packet_id"],))
    record = f"Hospital: {req.hospitalName} | Group: {pkt['blood_group']} | Packet ID: {pkt['packet_id']} | Urgency: {tier}"
    cur.execute("INSERT INTO blood_history (record) VALUES (%s)", (record,))
    conn.commit()
    cur.close()
    conn.close()

    return {
        "matchType": match_type,
        "priority": tier,
        "packet": {"packetId": pkt["packet_id"], "bloodGroup": pkt["blood_group"]}
    }