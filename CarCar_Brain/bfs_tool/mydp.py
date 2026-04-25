import mybfs
import pandas
from collections import deque
import heapq

DIRS = ["north", "east", "south", "west"]

COST = {
    "f": 1,
    "l": 1.2,
    "r": 1.2,
    "b": 1.6
}

def turn_cost(prev, curr):
    if prev is None:
        return COST["f"], curr

    i, j = DIRS.index(prev), DIRS.index(curr)
    diff = (j - i) % 4

    if diff == 0:
        return COST["f"], curr
    elif diff == 1:
        return COST["r"], curr
    elif diff == 3:
        return COST["l"], curr
    else:
        return COST["b"], curr

def dijkstra_full(adj, start, start_dir):
    dist = {}
    parent = {}

    pq = []

    dist[(start, start_dir)] = 0
    heapq.heappush(pq, (0, start, start_dir))

    while pq:
        curr_cost, node, prev_dir = heapq.heappop(pq)

        if curr_cost > dist[(node, prev_dir)]:
            continue

        for nxt, move_dir in adj[node]:
            cost, new_dir = turn_cost(prev_dir, move_dir)
            new_cost = curr_cost + cost

            state = (nxt, new_dir)

            if state not in dist or new_cost < dist[state]:
                dist[state] = new_cost
                parent[state] = (node, prev_dir)
                heapq.heappush(pq, (new_cost, nxt, new_dir))

    return dist#, parent


def build_cost_table(adj, nodes):
    """
    nodes = [start] + targets

    cost[i][j][d1][d2]
    """

    n = len(nodes)
    cost = [[[[float('inf')] * 4 for _ in range(4)] for _ in range(n)] for _ in range(n)]
    sp_parents = {}  # key: (i, d1)

    for i, s in enumerate(nodes):
        for d1 in range(4):
            start_dir = DIRS[d1]

            dist = dijkstra_full(adj, s, start_dir)
            #sp_parents[(i, d1)] = parent   # store full parent map

            for j, t in enumerate(nodes):
                for d2 in range(4):
                    end_state = (t, DIRS[d2])

                    if end_state in dist:
                        cost[i][j][d1][d2] = dist[end_state]

                    

    return cost#, sp_parents

def dp_flrb_clean(adj, start, start_dir, targets, time_limit):
    nodes = [start] + targets
    n = len(targets)

    cost = build_cost_table(adj, nodes)

    INF = float('inf')
    size = 1 << n

    # dp[mask][i][d]
    dp = [[[INF] * 4 for _ in range(n)] for _ in range(size)]
    parent = {}

    # initialize (start → first target)
    for i in range(n):
        for d2 in range(4):
            best = INF
            best_d1 = None
            if (start_dir == -1) :
                for d1 in range(4):
                    c = cost[0][i + 1][d1][d2]
                    if c < best:
                        best = c
                        best_d1 = d1
            else :
                best_d1 = start_dir
                best = cost[0][i + 1][start_dir][d2]
            

            if best < INF:
                dp[1 << i][i][d2] = best
                parent[(1 << i, i, d2)] = (0, -1, best_d1)
                #start_choice[(1 << i, i, d2)] = best_d1

            

    # transitions
    for mask in range(size):
        for u in range(n):
            for d1 in range(4):
                if dp[mask][u][d1] == INF:
                    continue

                for v in range(n):
                    if mask & (1 << v):
                        continue

                    for d2 in range(4):
                        c = cost[u + 1][v + 1][d1][d2]
                        if c == INF:
                            continue

                        new_cost = dp[mask][u][d1] + c
                        new_mask = mask | (1 << v)

                        if new_cost < dp[new_mask][v][d2]:
                            dp[new_mask][v][d2] = new_cost
                            parent[(new_mask, v, d2)] = (mask, u, d1)

    # final answer
    full = (1 << n) - 1
    best_time = INF
    best_score = 0
    best_state = None

    for u in range(n):
        for d in range(4):
            if dp[full][u][d] < best:
                best = dp[full][u][d]
                best_state = (full, u, d)

    for mask in range(size):
        for u in range(n):
            if ((1 << u) & mask == 0) : continue
            for d in range(4):
                if dp[mask][u][d] < time_limit:
                    score = mybfs.mask_score(mask, targets)
                    if (score > best_score) or (score == best_score and dp[mask][u][d] < best_time) :
                        best_time = dp[mask][u][d]
                        best_score = score
                        best_state = (mask, u, d)

    return best_time, best_state, parent#, sp_parents


def reconstruct_order(parent, best_state):
    path = []
    state = best_state
    #print(state)
    while state[0] != 0:
        mask, u, d = state
        path.append((u, d));
        state = parent[state]
        #print(state)

    path.reverse()
    return path, state[2] # path/ start_dir

def getorder(adj, start, start_dir, targets, time_limit) :
    best, best_state, parent = dp_flrb_clean(adj, start, start_dir, targets, time_limit)
    path, start_dir = reconstruct_order(parent, best_state);
    return path, start_dir

def test():
    raw_data = pandas.read_csv('medium_maze.csv').values
    mybfs.init(3, 4, 1)
    adj = mybfs.build_adjacency_list(raw_data)

    targets = [7, 9, 10, 12]
    start = 1
    #targets = [1, 6, 7, 12, 19, 24, 30, 31, 36, 43, 45, 48]
    #start = 25
    best_time, best_state, parent = dp_flrb_clean(adj, start, -1, targets, 100)
    print("cost:", best_time)
    print("points:", mybfs.mask_score(best_state[0], targets))
    path, start_dir = reconstruct_order(parent, best_state);
    path, start_dir = getorder(adj, start, -1, targets, 100)
    print("path: ")
    for (u, d) in path:
        print(targets[u], d)

    last, last_dir = path[0]
    print(0, targets[last], DIRS[start_dir], ": ", end = "")
    directions = mybfs.bfs_directions(adj, start, DIRS[start_dir], targets[last])
    print(mybfs.convert_to_commands(directions, DIRS[start_dir]))

    for (i, d) in path[1:]:
        print(targets[last], targets[i], DIRS[last_dir], ": ", end = "")
        directions = mybfs.bfs_directions(adj, targets[last], DIRS[last_dir], targets[i])
        print(mybfs.convert_to_commands(directions, DIRS[last_dir]))
        #print(mybfs.bfs_directions(adj, targets[last], DIRS[last_dir], targets[i]))
        last = i
        last_dir = d
        print("last_dir:", d)

test()