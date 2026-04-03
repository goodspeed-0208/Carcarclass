from hm10_esp32 import HM10ESP32Bridge
import time
import sys
import threading

PORT = 'COM7'
EXPECTED_NAME = 'HM10_12'

def background_listener(bridge):
    while True:
        msg = bridge.listen()
        if msg:
            if (msg == "finish init") :
                print("send")
                bridge.send("receive init")
            print(f"\r[HM10]: {msg}")
            print("You: ", end="", flush=True)
        time.sleep(0.02)

def main():
    bridge = HM10ESP32Bridge(port=PORT)
    
    # 1. Configuration Check
    current_name = bridge.get_hm10_name()
    if current_name != EXPECTED_NAME:
        print(f"Target mismatch. Current: {current_name}, Expected: {EXPECTED_NAME}")
        print(f"Updating target name to {EXPECTED_NAME}...")
        
        if bridge.set_hm10_name(EXPECTED_NAME):
            print("✅ Name updated successfully. Resetting ESP32...")
            bridge.reset()
            # Re-init after reset
            bridge = HM10ESP32Bridge(port=PORT)
        else:
            print("❌ Failed to set name. Exiting.")
            sys.exit(1)

    # 2. Connection Check
    status = bridge.get_status()
    if status != "CONNECTED":
        print(f"⚠️ ESP32 is {status}. Please ensure HM-10 is advertising. Exiting.")
        sys.exit(0)

    print(f"✨ Ready! Connected to {EXPECTED_NAME}")
    threading.Thread(target=background_listener, args=(bridge,), daemon=True).start()

    try:
        while True:
            user_msg = input("You: ")
            if user_msg.lower() in ['exit', 'quit']: break
            if user_msg: bridge.send(user_msg)
    except (KeyboardInterrupt, EOFError):
        pass
    
    print("\nChat closed.")

if __name__ == "__main__":
    main()