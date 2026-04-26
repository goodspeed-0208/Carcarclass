import mybfs
import pandas
from collections import deque
import heapq

DIRS = ["north", "east", "south", "west"]
# 150 200 add minus innode back
# foward left right turn back lb rb 
track_time = {
    "150": 0.573,
    "200": 0.57983,
    "add": 0.5312,
    "minus": 0.6122,
    "innode": 0.48332,
    "back": 0.443
}

# Second dictionary with action keys
turn_time = {
    "F": 0.18655,
    "L": 0.5038,
    "R": 0.49242,
    "T": 0.7903,
    "B": 1.0005,
    "LB": 0.488,
    "RB": 0.4505
}

COST = {
    "ff": track_time["200"]+turn_time["F"], "fr": track_time["minus"]+turn_time["R"], "fl": track_time["minus"]+turn_time["L"], "fb": track_time["innode"],
    "rf": track_time["add"]+turn_time["F"], "rr": track_time["150"]+turn_time["R"], "rl": track_time["150"]+turn_time["L"], "rb": track_time["innode"],
    "lf": track_time["add"]+turn_time["F"], "lr": track_time["150"]+turn_time["R"], "ll": track_time["150"]+turn_time["L"], "lb": track_time["innode"],
    "bf": turn_time["T"]+track_time["back"]+turn_time["F"], "br": turn_time["B"]+turn_time["RB"], "bl": turn_time["B"]+turn_time["LB"], "bb": 100000
}

def get_relative_move(prev_dir, curr_dir):
    i, j = DIRS.index(prev_dir), DIRS.index(curr_dir)
    diff = (j - i) % 4

    if diff == 0:
        return "f"
    elif diff == 1:
        return "r"
    elif diff == 3:
        return "l"
    else:
        return "b"

def turn_cost(prev_dir, curr_dir, prev_move):
    curr_move = get_relative_move(prev_dir, curr_dir)

    if prev_move is None:
        return COST["f" + curr_move], curr_dir, curr_move

    key = prev_move + curr_move
    return COST[key], curr_dir, curr_move

def dijkstra_full(adj, start, start_dir):
    dist = {}
    pq = []

    # state: (node, dir, first_move, last_move)
    start_state = (start, start_dir, None, None)
    dist[start_state] = 0

    heapq.heappush(pq, (0, start, start_dir, None, None))

    while pq:
        curr_cost, node, prev_dir, first_move, last_move = heapq.heappop(pq)

        if curr_cost > dist[(node, prev_dir, first_move, last_move)]:
            continue

        for nxt, move_dir in adj[node]:
            curr_move = get_relative_move(prev_dir, move_dir)

            new_first = curr_move if first_move is None else first_move

            if last_move is None:
                cost = 0#COST["f" + curr_move]
            else:
                cost = COST[last_move + curr_move]

            new_cost = curr_cost + cost
            state = (nxt, move_dir, new_first, curr_move)

            if state not in dist or new_cost < dist[state]:
                dist[state] = new_cost
                heapq.heappush(pq, (new_cost, nxt, move_dir, new_first, curr_move))

    return dist


def build_cost_table(adj, nodes):
    n = len(nodes)

    INF = float('inf')

    cost = [[[[[INF]*4 for _ in range(4)] for _ in range(4)] for _ in range(n)] for _ in range(n)]
    first_move = [[[[[None]*4 for _ in range(4)] for _ in range(4)] for _ in range(n)] for _ in range(n)]

    for i, s in enumerate(nodes):
        for d1 in range(4):
            start_dir = DIRS[d1]

            dist = dijkstra_full(adj, s, start_dir)

            for (node, d_end, first_m, last_m), c in dist.items():
                if node not in nodes:
                    continue

                j = nodes.index(node)
                d2 = DIRS.index(d_end)
                if (last_m == None) : m2 = 0
                else : m2 = "frlb".index(last_m)

                if c < cost[i][j][d1][d2][m2]:
                    cost[i][j][d1][d2][m2] = c
                    first_move[i][j][d1][d2][m2] = first_m

    return cost, first_move



def dp_flrb_clean(adj, start, start_dir, start_move, targets, time_limit):
    nodes = [start] + targets
    n = len(targets)

    cost, first_move = build_cost_table(adj, nodes)

    INF = float('inf')
    size = 1 << n

    # dp[mask][i][d][m]
    dp = [[[[INF]*4 for _ in range(4)] for _ in range(n)] for _ in range(size)]
    parent = {}

    move_idx = {"f":0, "r":1, "l":2, "b":3}
    moves = ["f","r","l","b"]

    # ===== 初始化 =====
    for i in range(n):
        for d2 in range(4):
            for m2 in range(4):

                if start_dir == -1:
                    for d1 in range(4):
                        c = cost[0][i+1][d1][d2][m2]
                        if c == INF:
                            continue

                        fm = first_move[0][i+1][d1][d2][m2]

                        if start_move is None:
                            extra = COST["f"+fm]
                        else:
                            extra = COST[start_move + fm]

                        new_cost = c + extra

                        if new_cost < dp[1<<i][i][d2][m2]:
                            dp[1<<i][i][d2][m2] = new_cost
                            parent[(1<<i, i, d2, m2)] = (0, -1, d1, start_move)

                else:
                    c = cost[0][i+1][start_dir][d2][m2]
                    if c == INF:
                        continue

                    fm = first_move[0][i+1][start_dir][d2][m2]

                    if start_move is None:
                        extra = COST["f"+fm]
                    else:
                        extra = COST[start_move + fm]

                    new_cost = c + extra

                    if new_cost < dp[1<<i][i][d2][m2]:
                        dp[1<<i][i][d2][m2] = new_cost
                        parent[(1<<i, i, d2, m2)] = (0, -1, start_dir, start_move)

    # ===== transitions =====
    for mask in range(size):
        for u in range(n):
            for d1 in range(4):
                for m1 in range(4):

                    if dp[mask][u][d1][m1] == INF:
                        continue

                    prev_move = moves[m1]

                    for v in range(n):
                        if mask & (1<<v):
                            continue

                        for d2 in range(4):
                            for m2 in range(4):

                                c = cost[u+1][v+1][d1][d2][m2]
                                if c == INF:
                                    continue

                                fm = first_move[u+1][v+1][d1][d2][m2]

                                extra = COST[prev_move + fm]

                                new_cost = dp[mask][u][d1][m1] + c + extra
                                new_mask = mask | (1<<v)

                                if new_cost < dp[new_mask][v][d2][m2]:
                                    dp[new_mask][v][d2][m2] = new_cost
                                    parent[(new_mask, v, d2, m2)] = (mask, u, d1, m1)

    # ===== 選答案 =====
    best_time = INF
    best_score = 0
    best_state = None

    for mask in range(size):
        for u in range(n):
            if not (mask & (1<<u)):
                continue
            for d in range(4):
                for m in range(4):

                    if dp[mask][u][d][m] < time_limit:
                        score = mybfs.mask_score(mask, targets)

                        if (score > best_score) or (
                            score == best_score and dp[mask][u][d][m] < best_time
                        ):
                            best_time = dp[mask][u][d][m]
                            best_score = score
                            best_state = (mask, u, d, m)

    return best_time, best_state, parent

def reconstruct_order(parent, best_state):
    path = []
    state = best_state

    while state[0] != 0:
        mask, u, d, m = state
        path.append((u, d))
        state = parent[state]

    path.reverse()
    return path, state[2]

def getorder(adj, start, start_dir, start_move, targets, time_limit):
    best, best_state, parent = dp_flrb_clean(
        adj, start, start_dir, start_move, targets, time_limit
    )
    path, start_dir = reconstruct_order(parent, best_state)
    return best, path, start_dir


def test():
    raw_data = pandas.read_csv('big_maze_114.csv').values
    mybfs.init(6, 8, 25)
    adj = mybfs.build_adjacency_list(raw_data)

    targets = [1, 6, 7, 12, 19, 24, 30, 31, 36, 43, 45, 48]
    start = 25

    start_move = None
    best_time, path, start_dir = getorder(adj, start, -1, start_move, targets, 65)

    last, last_dir = path[0]

    print(start, targets[last], DIRS[start_dir], ": ", end="")
    directions = mybfs.bfs_directions(adj, start, DIRS[start_dir], targets[last])
    print(mybfs.convert_to_commands(directions, DIRS[start_dir]))

    for (i, d) in path[1:]:
        print(targets[last], targets[i], DIRS[last_dir], ": ", end="")
        directions = mybfs.bfs_directions(
            adj, targets[last], DIRS[last_dir], targets[i]
        )
        print(mybfs.convert_to_commands(directions, DIRS[last_dir]))

        last = i
        last_dir = d
    print("time required:", best_time)

test()