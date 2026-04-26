from hm10_esp32 import HM10ESP32Bridge
import mybfs
import mydp
import mydp_new
import pandas
import time
import sys
import threading
import score
import re

PORT = 'COM3'
EXPECTED_NAME = 'HM10_12'

raw_data = pandas.read_csv('medium_maze.csv').values
adj = mybfs.build_adjacency_list(raw_data)
row = 3
column = 4
start = 1
mybfs.init(row, column, start)
targets = [ 7,9,10, 12]
scoreboard = None
scoreboard = score.ScoreboardServer("GOODSPEED", "http://140.112.175.18")
#print(adj)
last = None

DIRS = ["north", "east", "south", "west"]

# 用 state 來存目前狀態
state = {
    "curpos": start,
    "curdir": ""   # 這裡要改成你的車一開始真正面向的方向
}

lastmove = None
next_target = None

def senddirmsg(bridge, state):
    if (len(targets) == 0) : return
    curpos = state["curpos"]
    curdir = state["curdir"]
    #print("curpos", curpos)
    #print("curdir", curdir)
    global scoreboard
    remaining_time = scoreboard.getTime()
    print("Remaining time:", remaining_time)
    global lastmove
    global next_target

    directions = mybfs.bfs_directions(adj, curpos, DIRS[curdir], next_target)
    commands = mybfs.convert_to_commands(directions, DIRS[curdir])
    if not directions: #find target
        print("ready to get target:", next_target)
        targets.remove(next_target)
        if (len(targets) == 0) : #end
            print("send:stop")
            bridge.send("Dir:stop\n")
            return
        else :
            best_time, path, start_dir = mydp_new.getorder(adj, curpos, curdir, lastmove, targets, remaining_time)
            next_target = targets[path[0][0]]
            directions = mybfs.bfs_directions(adj, curpos, DIRS[curdir], next_target)
            commands = mybfs.convert_to_commands(directions, DIRS[curdir])

    if (commands[0] == 'b') :
        if (len(commands) > 1 and commands[1] == 'f') :
            commands = "t"

    global last
    print("target:", next_target)
    if (last == 'b' and (commands[0] == 'r' or commands[0] == 'l')):
        print("send:b", commands[0])
        bridge.send("Dir:"+commands[0]+"b\n")
    else:
        print("send:", commands[0])
        bridge.send("Dir:" + commands[0] + "\n")
        
    last = commands[0]
    # 更新狀態
    state["curpos"] = mybfs.move(curpos, directions[0])
    state["curdir"] = DIRS.index(directions[0])

    #print("new curpos:", state["curpos"])
    #print("new curdir:", state["curdir"])
    #print("targets:", targets)



def handle_uid(uid) :
    global scoreboard
    scoreboard.add_UID(uid)
    print("get_UID:", uid)

def background_listener(bridge, state):
    while True:
        # 使用內圈 while，確保緩衝區裡拼好的完整行全部讀完
        while True:
            msg = bridge.listen()
            if not msg:
                break # 沒有完整的行了，跳出內圈去睡覺
                
            print(f"{msg}") #[HM10]:

            if "INN" in msg:
                senddirmsg(bridge, state)
            
            match = re.search(r"uid:\s*([0-9A-Fa-f]{8})", msg)
            if match:
                uid = match.group(1)
                print("UID detected:", uid)

                handle_uid(uid)


            

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
    global scoreboard
    scoreboard = score.ScoreboardServer("GOODSPEED", "http://140.112.175.18")

    print("start =", start)
    state["curpos"] = start

    best_time, path, start_dir = mydp_new.getorder(adj, start, -1, None, targets, 10000)#scoreboard.getTime())
    print("start_dir", start_dir)
    global next_target
    next_target = targets[path[0][0]]
    directions = mybfs.bfs_directions(adj, start, DIRS[start_dir], next_target)
    state["curpos"] = mybfs.move(start, directions[0])
    state["curdir"] = DIRS.index(directions[0])
    global lastmove
    lastmove = "f"
    
    print("initial curpos =", state["curpos"])
    print("initial curdir =", state["curdir"])
    print("target:", next_target)

    threading.Thread(
        target=background_listener,
        args=(bridge, state),
        daemon=True
    ).start()

    try:
        while True:
            user_msg = input("YOU: ")
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
