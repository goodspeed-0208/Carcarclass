from hm10_esp32 import HM10ESP32Bridge
import mybfs
import mydp
import pandas
import time
import sys
import threading
import score

PORT = 'COM3'
EXPECTED_NAME = 'HM10_12'

raw_data = pandas.read_csv('big_maze_114.csv').values
adj = mybfs.build_adjacency_list(raw_data)
row = 6
column = 8
mybfs.setsize(row, column)
targets = [1, 6, 7, 12, 19, 24, 30, 31, 36, 43, 45, 48]
start = 25
print(adj)

DIRS = ["north", "east", "south", "west"]

start = 1
targets = [2, 3, 4, 5, 6]
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
    path, start_dir = mydp.getorder(adj, curpos, targets)
    directions = mybfs.bfs_directions(adj, curpos, DIRS[start_dir], targets[path[0][0]])
    if not directions: #find target
        print("ready to get target:", targets[path[0][0]])
        targets.remove(path[0][0])
        if (len(targets) == 0) : #end
            print("send:stop")
            bridge.send("Dir:stop\n")
            return
        else :
            path, start_dir = mydp.getorder(adj, curpos, targets)
            directions = mybfs.bfs_directions(adj, curpos, DIRS[start_dir], targets[path[0][0]])
    

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
        # 使用內圈 while，確保緩衝區裡拼好的完整行全部讀完
        while True:
            msg = bridge.listen()
            if not msg:
                break # 沒有完整的行了，跳出內圈去睡覺
                
            print(f"[HM10]: {msg}")

            if "outn" in msg:
                senddirmsg(bridge, state)

        time.sleep(0.002)


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
    state["curpos"] = start
    state["curdir"] = (mydp.getorder(adj, start, targets))[1]
    state["curpos"] = mybfs.move(state["curpos"], state["curdir"])

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