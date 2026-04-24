
import numpy as np
import pandas
from collections import defaultdict
import math
from collections import deque


#raw_data = pandas.read_csv('maze.csv').values
row = 6
column = 8
start = 25

def init(r, c, s) :
    global row
    row = r
    global column
    column = c
    global start
    start = s


def build_adjacency_list(raw_data):
    adj = defaultdict(list)

    direction_labels = ["north", "south", "west", "east"]

    for i, row in enumerate(raw_data, start=1):
        neighbors = row[1:5]

        for direction, neighbor in zip(direction_labels, neighbors):
            # skip if neighbor is NaN
            if neighbor is None or (isinstance(neighbor, float) and math.isnan(neighbor)):
                continue

            neighbor = int(neighbor)

            adj[i].append((neighbor, direction))

    return adj




#adj = build_adjacency_list(raw_data)

#print(dict(adj))

def convert_to_commands(directions, start_dir):
    order = ["north", "east", "south", "west"]
    commands = []

    prev = start_dir
    if (start_dir == -1) : start_dir = directions[0]
    #prev = directions[0]  # assume initial orientation = first move
    #commands.append("f")

    for curr in directions:
        prev_idx = order.index(prev)
        curr_idx = order.index(curr)

        diff = (curr_idx - prev_idx) % 4

        if diff == 0:
            commands.append("f")
        elif diff == 1:
            commands.append("r")
        elif diff == 3:
            commands.append("l")
        elif diff == 2:
            commands.append("t")

        prev = curr

    return "".join(commands)

def bfs_directions(adj, start, start_dir, goal):
    queue = deque()
    queue.append((start, [], None))  
    # (current_node, path_directions, current_heading)

    visited = set()

    while queue:
        node, path, heading = queue.popleft()

        if node == goal:
            return path
            #return convert_to_commands(path, start_dir)

        if node in visited:
            continue
        visited.add(node)

        for neighbor, direction in adj[node]:
            if neighbor not in visited:
                queue.append((neighbor, path + [direction], direction))

    return None

def move(curpos, dir) :
    if (dir == "north") : curpos -= 1
    elif (dir == "south") : curpos += 1
    elif (dir == "west") : curpos += row
    elif (dir == "east") : curpos -= row
    return curpos

def mask_score(mask, targets) :
    start_pos = ((start-1)//row, (start-1)%row)
    score = 0
    for i in range(len(targets)):
        if (mask & (1 << i) == 0) : continue
        node = targets[i]
        score += abs((node-1)//row - start_pos[0]) + abs((node-1)%row - start_pos[1])
    return score
#print("commands: " + bfs_directions(adj, 2, 12))

