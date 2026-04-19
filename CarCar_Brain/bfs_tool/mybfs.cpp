#include<bits/stdc++.h>
using namespace std;

string bfs(int n, int m, vector<vector<int> >v, int start, int end) {
	vector<int>parent(n*m+1, -1); // also record from
	queue<int> q;
	q.push(start);
	parent[start] = 0;
	while (!q.empty()) {
		int x = q.front();
		q.pop();
		for (auto i : v[x]) {
			if (parent[i] != -1) continue;
			parent[i] = x;
			q.push(i);
		}
	}
	vector<int>dir;
	while (end != start) {
		int from = parent[end];
		if (end == from-1) //down
			dir.push_back(0);
		else if (end == from+1) // up
			dir.push_back(2);
		else if (end == from-n) // right
			dir.push_back(1);
		else if (end == from+n) // left
			dir.push_back(3);
		else exit(-1);
		end = from;
	}
	reverse(dir.begin(), dir.end());
	string s = "";
	s += "f";
	for (int i = 1 ; i < dir.size() ; i++) {
		int turn = (dir[i] - dir[i-1] + 4) % 4;
		if (turn == 1) s += "r";
		else if (turn == 2) s += "b";
		else if (turn == 3) s += "l";
		else s += "f";
	}
	return s;
}

signed main() {
    
}