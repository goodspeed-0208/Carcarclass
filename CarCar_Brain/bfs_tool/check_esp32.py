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
scoreboard = None

raw_data = pandas.read_csv('big_maze_114.csv').values
adj = mybfs.build_adjacency_list(raw_data)
row = 6
column = 8
start = 25
mybfs.init(row, column, start)
targets = [1, 6, 7, 12, 19, 24, 30, 31, 36, 43, 45, 48]
#print(adj)

DIRS = ["north", "east", "south", "west"]

# 用 state 來存目前狀態
state = {
    "curpos": start,
    "curdir": ""   # 這裡要改成你的車一開始真正面向的方向
}

lastmove = None
next_target = None

def senddirmsg(state):
    curpos = state["curpos"]
    curdir = state["curdir"]

    remaining_time = scoreboard.getTime()
    print("Remaining time:", remaining_time)
    global lastmove
    global next_target
    #best_time, path, start_dir = mydp_new.getorder(adj, curpos, curdir, lastmove, next_target, remaining_time)
    directions = mybfs.bfs_directions(adj, curpos, DIRS[curdir], next_target)
    commands = mybfs.convert_to_commands(directions, DIRS[curdir])
    if not directions: #find target
        print("ready to get target:", next_target)
        targets.remove(next_target)
        if (len(targets) == 0) : #end
            print("send:stop")
            return
        else :
            best_time, path, start_dir = mydp_new.getorder(adj, curpos, curdir, lastmove, targets, remaining_time)
            next_target = targets[path[0][0]]
            directions = mybfs.bfs_directions(adj, curpos, DIRS[curdir], next_target)
            commands = mybfs.convert_to_commands(directions, DIRS[curdir])


    print("target:", next_target)
    #if (last == 'b' and (command[1] == 'r' or command[1] == 'l')):
        #print("send:b", command[1])
    #else:
    print("send:", commands[0])
    #print("expected time", best_time)
    lastmove = commands[0]
        #last = command[1]

    # 更新狀態
    state["curpos"] = mybfs.move(curpos, directions[0])
    state["curdir"] = DIRS.index(directions[0])

    print("new curpos:", state["curpos"])
    print("new curdir:", state["curdir"])
    #print("targets:", targets)

def handle_uid(uid) :
    scoreboard.add_UID(uid)
    print("get_UID:", uid)

def main():
    
    print(f"✨ Ready! Connected to {EXPECTED_NAME}")
    global scoreboard
    scoreboard = score.ScoreboardServer("GOODSPEED", "http://140.112.175.18")

    print("start =", start)
    state["curpos"] = start
    #state["curdir"] = (mydp_new.getorder(adj, start, -1, None, targets, scoreboard.getTime()))[2]
    
    
    
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
    
    while scoreboard.getTime() > 0 :
        c = input()
        if c == "e" : break
        else :
            senddirmsg(state)

if __name__ == "__main__":
    main()

