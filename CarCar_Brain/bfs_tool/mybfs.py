
import numpy as np
import pandas
from collections import defaultdict
import math
from collections import deque


raw_data = pandas.read_csv('maze.csv').values
row = 3, column = 4

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




adj = build_adjacency_list(raw_data)

#print(dict(adj))

def convert_to_commands(directions):
    order = ["north", "east", "south", "west"]
    commands = []

    prev = directions[0]  # assume initial orientation = first move
    commands.append("f")

    for curr in directions[1:]:
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
            commands.append("b")

        prev = curr

    return "".join(commands)

def bfs_directions(adj, start, goal):
    queue = deque()
    queue.append((start, [], None))  
    # (current_node, path_directions, current_heading)

    visited = set()

    while queue:
        node, path, heading = queue.popleft()

        if node == goal:
            return path
            #return convert_to_commands(path)

        if node in visited:
            continue
        visited.add(node)

        for neighbor, direction in adj[node]:
            queue.append((neighbor, path + [direction], direction))

    return None

def move(curpos, dir) :
    if (dir == "north") : curpos -= 1
    elif (dir == "south") : curpos += 1
    elif (dir == "west") : curpos += column
    elif (dir == "east") : curpos -= column
    return curpos



print(bfs_directions(adj, 2, 12))

