import mybfs
import pandas
from collections import deque
import heapq



DIRS = ["north", "east", "south", "west"]

COST = {
    "f": 1,
    "l": 1,
    "r": 1,
    "b": 100
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

def dp_flrb_clean(adj, start, targets):
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

            for d1 in range(4):
                c = cost[0][i + 1][d1][d2]
                if c < best:
                    best = c
                    best_d1 = d1

            if best < INF:
                dp[1 << i][i][d2] = best
                parent[(1 << i, i, d2)] = None
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
    best = INF
    best_state = None

    for u in range(n):
        for d in range(4):
            if dp[full][u][d] < best:
                best = dp[full][u][d]
                best_state = (full, u, d)

    return best, best_state, parent#, sp_parents
'''
def reconstruct_sp(sp_parent, end_state):
    path = []
    state = end_state

    while state in sp_parent:
        path.append(state)
        state = sp_parent[state]

    path.append(state)
    path.reverse()

    return path
'''
def reconstruct_order(parent, best_state):
    path = []
    state = best_state
    print(state)
    while state is not None:
        mask, u, d = state
        path.append((mask, u));
        state = parent[state]
        print(state)

    path.reverse()
    return path
'''
def reconstruct_full_path(start, targets, order, sp_parents):
    nodes = [start] + targets

    full_path = []
    prev_node_idx = 0  # start index
    best, best_state, dp_parent, sp_parents = dp_flrb_clean(adj, start, targets)
    _, _, start_dir = dp_parent[best_state]
    prev_dir = DIRS[start_dir]

    for (target_idx, end_dir) in order:
        sp_parent = sp_parents[(prev_node_idx, DIRS.index(prev_dir))]

        end_state = (nodes[target_idx + 1], DIRS[end_dir])

        segment = reconstruct_sp(sp_parent, end_state)

        if full_path:
            segment = segment[1:]  # avoid duplicate node

        full_path.extend(segment)

        prev_node_idx = target_idx + 1
        prev_dir = DIRS[end_dir]

    return full_path
'''

raw_data = pandas.read_csv('medium_maze.csv').values
mybfs.setsize(3, 4)
raw_data = pandas.read_csv('big_maze_114.csv').values
mybfs.setsize(6, 8)
adj = mybfs.build_adjacency_list(raw_data)
#print(adj)
#targets = [1, 2, 3, 4, 6, 8, 11, 7, 9, 10, 12]
#best, best_state, parent = dp_flrb_clean(adj, 5, targets)
targets = [1, 6, 7, 12, 19, 24, 30, 31, 36, 43, 45, 48]
best, best_state, parent = dp_flrb_clean(adj, 25, targets)
print("//////")
print(best)
path = reconstruct_order(parent, best_state);
for (mask, i) in path:
    print(mask, targets[i])