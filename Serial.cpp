#include <bits/stdc++.h>
using namespace std;

// Generate Parent1 according to Algorithm 1 in Kao et al.
vector<int> Swap(const vector<int>& v, int x) {
    int n = v.size();
    vector<int> p = v;
    // find position i where v[i] == x
    int i = find(p.begin(), p.end(), x) - p.begin();
    if (i + 1 < n) {
        swap(p[i], p[i+1]);
    }
    return p;
}

int r_pos(const vector<int>& v) {
    // position of first symbol from right not in right position
    int n = v.size();
    for (int i = n - 1; i >= 0; --i) {
        if (v[i] != i+1) return i + 1;  // 1-based symbol
    }
    return 1;
}

vector<int> FindPosition(const vector<int>& v, int t, const vector<int>& root) {
    int n = v.size();
    auto s_t = Swap(v, t);
    if (t == 2 && s_t == root) {
        return Swap(v, t-1);
    } else if (v[n-2] == t || v[n-2] == n-1) {
        int j = r_pos(v);
        return Swap(v, j);
    } else {
        return Swap(v, t);
    }
}

vector<int> Parent1(const vector<int>& v, int t, const vector<int>& root) {
    int n = v.size();
    if (v[n-1] == n) {
        if (t != n-1) {
            return FindPosition(v, t, root);
        } else {
            return Swap(v, v[n-2]);
        }
    } else {
        if (v[n-1] == n-1 && v[n-2] == n && Swap(v, n) != root) {
            if (t == 1) {
                return Swap(v, n);
            } else {
                return Swap(v, t-1);
            }
        } else {
            if (v[n-1] == t) {
                return Swap(v, n);
            } else {
                return Swap(v, t);
            }
        }
    }
}

int main() {
    int n;
    cout << "Enter n: ";
    cin >> n;
    vector<int> base(n);
    iota(base.begin(), base.end(), 1);
    vector<vector<int>> perms;
    do {
        perms.push_back(base);
    } while (next_permutation(base.begin(), base.end()));

    int N = perms.size();
    // map perm to index
    map<vector<int>, int> idx;
    for (int i = 0; i < N; ++i) {
        idx[perms[i]] = i;
    }

    vector<int> root = perms[0];

    // For each tree T_t, build parent array and children lists
    for (int t = 1; t <= n-1; ++t) {
        vector<int> parent(N, -1);
        vector<vector<int>> children(N);
        parent[0] = -1; // root has no parent
        for (int i = 1; i < N; ++i) {
            vector<int> p = Parent1(perms[i], t, root);
            int pi = idx[p];
            parent[i] = pi;
            children[pi].push_back(i);
        }
        // BFS for level-order traversal
        cout << "Level-order traversal of IST T" << t << ":\n";
        queue<int> q;
        q.push(0);
        while (!q.empty()) {
            int sz = q.size();
            for (int i = 0; i < sz; ++i) {
                int u = q.front(); q.pop();
                for (int x : perms[u]) cout << x << ' ';
                if (i < sz - 1) cout << "| ";
                // enqueue children
                for (int c : children[u]) q.push(c);
            }
            cout << "|\n";
        }
        cout << '\n';
    }
    return 0;
}
