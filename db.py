import os
import psycopg2
from dotenv import load_dotenv

load_dotenv()

DATABASE_URL = os.getenv("DATABASE_URL")


def get_connection():
    if not DATABASE_URL:
        raise RuntimeError("DATABASE_URL is not set in environment variables")
    return psycopg2.connect(DATABASE_URL)


def init_db():
    conn = get_connection()
    cur = conn.cursor()

    cur.execute("""
        CREATE TABLE IF NOT EXISTS users (
            student_id VARCHAR(20) PRIMARY KEY,
            name VARCHAR(100) NOT NULL,
            email VARCHAR(120) UNIQUE NOT NULL,
            mobile VARCHAR(15) NOT NULL,
            blood_group VARCHAR(10) DEFAULT 'O+',
            password VARCHAR(100) NOT NULL
        );
    """)

    cur.execute("""
        CREATE TABLE IF NOT EXISTS items (
            id SERIAL PRIMARY KEY,
            owner_id VARCHAR(20) REFERENCES users(student_id),
            title VARCHAR(150) NOT NULL,
            category VARCHAR(50) DEFAULT 'General',
            condition VARCHAR(50) DEFAULT 'Good',
            expected_return VARCHAR(150) DEFAULT 'Open to offers',
            status VARCHAR(20) DEFAULT 'OPEN',
            accepted_bidder_id VARCHAR(20),
            handshake_code VARCHAR(10)
        );
    """)

    # Auto-add columns in case old table existed
    cur.execute("ALTER TABLE items ADD COLUMN IF NOT EXISTS accepted_bidder_id VARCHAR(20);")
    cur.execute("ALTER TABLE items ADD COLUMN IF NOT EXISTS handshake_code VARCHAR(10);")

    cur.execute("""
        CREATE TABLE IF NOT EXISTS offers (
            id SERIAL PRIMARY KEY,
            item_id INT REFERENCES items(id),
            bidder_id VARCHAR(20) REFERENCES users(student_id),
            offer_text TEXT NOT NULL,
            status VARCHAR(20) DEFAULT 'PENDING'
        );
    """)

    cur.execute("""
        CREATE TABLE IF NOT EXISTS blood_stock (
            blood_group VARCHAR(10) PRIMARY KEY,
            units INT NOT NULL
        );
    """)

    blood_groups = ["A+", "A-", "B+", "B-", "AB+", "AB-", "O+", "O-"]
    for bg in blood_groups:
        cur.execute("""
            INSERT INTO blood_stock (blood_group, units)
            VALUES (%s, 10)
            ON CONFLICT (blood_group) DO NOTHING;
        """, (bg,))

    conn.commit()
    cur.close()
    conn.close()


if DATABASE_URL:
    init_db()