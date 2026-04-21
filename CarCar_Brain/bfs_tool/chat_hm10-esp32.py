from hm10_esp32 import HM10ESP32Bridge
import mybfs
import pandas
import time
import sys
import threading
import score

PORT = 'COM3'
EXPECTED_NAME = 'HM10_12'

raw_data = pandas.read_csv('maze.csv').values
adj = mybfs.build_adjacency_list(raw_data)
print(adj)

row = 3
column = 2
start = 1
end = 3
lastcommand = "f"
scoreboard = score.ScoreboardServer("REAL GOODSPEED", "http://140.112.175.18")

# 用 state 來存目前狀態
state = {
    "curpos": start,
    "curdir": ""   # 這裡要改成你的車一開始真正面向的方向
}

def senddirmsg(bridge, state):
    curpos = state["curpos"]
    curdir = state["curdir"]

    remaining_time = score.getTime()
    print(remaining_time)
    directions = mybfs.bfs_directions(adj, curpos, end)
    if not directions: #end
        print("send:stop")
        bridge.send("Dir:stop\n")
        return

    # directions[0] 是下一步方向
    command = mybfs.convert_to_commands([curdir] + directions)

    print("send:", command[1])
    bridge.send("Dir:" + command[1] + "\n")

    # 更新狀態
    state["curpos"] = mybfs.move(curpos, directions[0])
    state["curdir"] = directions[0]

    print("new curpos:", state["curpos"])
    print("new curdir:", state["curdir"])


def background_listener(bridge, state):
    while True:
        msg = bridge.listen()
        if msg:
            print("[HM10]:", msg)

            if msg == "outnode":
                senddirmsg(bridge, state)

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
    print("start =", start)
    print("initial curpos =", state["curpos"])
    print("initial curdir =", state["curdir"])

    directions = mybfs.bfs_directions(adj, state["curpos"], end)
    state["curpos"] = mybfs.move(state["curpos"], directions[0])
    state["curdir"] = directions[0]

    print("next curpos =", state["curpos"])
    print("next curdir =", state["curdir"])

    threading.Thread(
        target=background_listener,
        args=(bridge, state),
        daemon=True
    ).start()

    try:
        while True:
            user_msg = input("You: ")
            if user_msg.lower() in ['exit', 'quit']:
                break

            if user_msg:
                bridge.send(user_msg + "\n")

            if user_msg == "s":
                senddirmsg(bridge, state)

    except (KeyboardInterrupt, EOFError):
        pass

    print("\nChat closed.")


if __name__ == "__main__":
    main()