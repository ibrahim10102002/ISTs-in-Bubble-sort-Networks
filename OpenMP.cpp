#include <bits/stdc++.h>
#include <omp.h>                   // 1. include OpenMP header
using namespace std;

// … (Swap, r_pos, FindPosition, Parent1 as before) …

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

    // generate all permutations
    vector<int> base(n);
    iota(base.begin(), base.end(), 1);
    vector<vector<int>> perms;
    do { perms.push_back(base); }
    while (next_permutation(base.begin(), base.end()));

    int N = perms.size();
    unordered_map<string,int> idx;
    idx.reserve(N);
    for (int i = 0; i < N; ++i) {
        // stringify for fast lookup
        string key;
        for (int x: perms[i]) key += char('0'+x);
        idx[key] = i;
    }
    vector<int> root = perms[0];

    for (int t = 1; t <= n-1; ++t) {
        vector<int> parent(N, -1);
        vector<vector<int>> children(N);

        parent[0] = -1;  // root

        // 2. Parallelize parent computation
        #pragma omp parallel for schedule(static)
        for (int i = 1; i < N; ++i) {
            auto p = Parent1(perms[i], t, root);
            // reconstruct key
            string sk;
            for (int x: p) sk += char('0'+x);
            parent[i] = idx[sk];
        }

        // 3. Build children lists serially to avoid race conditions
        for (int i = 1; i < N; ++i) {
            children[parent[i]].push_back(i);
        }

        // 4. BFS traversal and printing (unchanged)
        cout << "Level-order traversal of IST T" << t << ":\n";
        queue<int> q; q.push(0);
        while (!q.empty()) {
            int sz = q.size();
            for (int i = 0; i < sz; ++i) {
                int u = q.front(); q.pop();
                for (int x : perms[u]) cout << x << ' ';
                if (i < sz-1) cout << "| ";
                for (int c : children[u]) q.push(c);
            }
            cout << "|\n";
        }
        cout << "\n";
    }
    return 0;
}
