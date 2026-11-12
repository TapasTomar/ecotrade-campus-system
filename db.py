import os
import psycopg2
from psycopg2.extras import RealDictCursor
from dotenv import load_dotenv
load_dotenv()

DATABASE_URL = os.getenv("DATABASE_URL", "")

def get_conn():
    if not DATABASE_URL:
        raise RuntimeError("DATABASE_URL environment variable is not set!")
    return psycopg2.connect(DATABASE_URL, cursor_factory=RealDictCursor)

def init_db():
    conn = get_conn()
    cur = conn.cursor()

    cur.execute("""
        CREATE TABLE IF NOT EXISTS students (
            student_id BIGINT PRIMARY KEY,
            name TEXT NOT NULL,
            roll_no TEXT NOT NULL,
            course TEXT NOT NULL,
            year INT NOT NULL,
            email TEXT NOT NULL,
            phone TEXT NOT NULL,
            blood_group TEXT NOT NULL,
            eco_points INT DEFAULT 10
        );
    """)

    cur.execute("""
        CREATE TABLE IF NOT EXISTS barter_items (
            id SERIAL PRIMARY KEY,
            owner_id BIGINT NOT NULL,
            name TEXT NOT NULL,
            condition TEXT NOT NULL,
            want_item TEXT NOT NULL,
            status TEXT DEFAULT 'AVAILABLE',
            buyer_id BIGINT DEFAULT 0,
            offer_item TEXT DEFAULT 'None',
            otp_code INT DEFAULT 0
        );
    """)

    cur.execute("""
        CREATE TABLE IF NOT EXISTS trade_history (
            id SERIAL PRIMARY KEY,
            record TEXT NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
    """)

    cur.execute("""
        CREATE TABLE IF NOT EXISTS blood_packets (
            packet_id SERIAL PRIMARY KEY,
            donor_id BIGINT NOT NULL,
            blood_group TEXT NOT NULL,
            center_name TEXT NOT NULL
        );
    """)

    cur.execute("""
        CREATE TABLE IF NOT EXISTS donor_cooldowns (
            donor_id BIGINT PRIMARY KEY,
            last_donation BIGINT NOT NULL
        );
    """)

    cur.execute("""
        CREATE TABLE IF NOT EXISTS blood_history (
            id SERIAL PRIMARY KEY,
            record TEXT NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
    """)

    conn.commit()
    cur.close()
    conn.close()