import argparse
import os
import requests
import time  # เพิ่ม time เข้ามาเพื่อทำ delay

DEFAULT_URL = "http://localhost:3883/pager"


def build_headers(secret: str | None):
    headers = {"Content-Type": "application/json"}
    if secret:
        headers["x-pager-secret"] = secret
    return headers

# กำหนดชุดข้อความสำหรับ Test ไว้ที่นี่
TEST_MESSAGES = [
    # "Test [DOWN]: DOWN",
    # "Test [UP]: UP",
    "Test [Down]: Down",
    "Test [Up]: Uown",
    "Test [ONLINE]: ONLINE",
    "Test [WARN]: WARN",
    "Test [LOVE]: LOVE"
]

def send(url: str, secret: str | None, message: str):
    try:
        r = requests.post(url, headers=build_headers(secret), json={"message": message})
        print(f"Sent: '{message}' -> Response: {r.json()}")
    except Exception as e:
        print(f"Error sending '{message}': {e}")


def run_list_test(url: str, secret: str | None):
    print(f"กำลังเริ่มส่งชุดข้อความทดสอบทั้งหมด {len(TEST_MESSAGES)} ข้อความ...")
    for msg in TEST_MESSAGES:
        send(url, secret, msg)
        time.sleep(0.5)  # เว้นจังหวะ 1.5 วินาทีก่อนส่งข้อความถัดไป (ปรับได้)
    print("ส่งชุดข้อความทดสอบเสร็จสิ้น")


def loop_input(url: str, secret: str | None):
    print("พิมพ์ข้อความแล้วกด Enter (Ctrl+C เพื่อออก)")
    while True:
        try:
            msg = input("> ").strip()
            if msg:
                send(url, secret, msg)
        except KeyboardInterrupt:
            print("\nBye")
            break


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-t", "--text", help="ข้อความที่จะส่ง")
    parser.add_argument("-l", "--list", action="store_true", help="ส่งชุดข้อความทดสอบอัตโนมัติ") # เพิ่ม argument ใหม่
    parser.add_argument("--url", default=os.getenv("PAGER_URL", DEFAULT_URL), help="Pager endpoint URL")
    parser.add_argument("--secret", default=os.getenv("PAGER_SECRET"), help="Pager secret header value")
    args = parser.parse_args()

    if args.list:
        run_list_test(args.url, args.secret)  # ถ้ารันด้วย -l ให้ส่ง list test
    elif args.text:
        send(args.url, args.secret, args.text)
    else:
        loop_input(args.url, args.secret)
