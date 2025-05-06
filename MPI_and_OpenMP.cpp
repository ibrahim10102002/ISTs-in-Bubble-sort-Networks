#include <bits/stdc++.h>
#include <mpi.h>
#include <omp.h>
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

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int n;
    vector<vector<int>> perms;
    if (rank == 0) {
        // read n and build perms on root
        cout << "Enter n: ";
        cin >> n;
        vector<int> base(n);
        iota(base.begin(), base.end(), 1);
        do { perms.push_back(base); }
        while (next_permutation(base.begin(), base.end()));
    }

    // broadcast n
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int N = 1;
    for (int i = 2; i <= n; ++i) N *= i;

    // flatten perms into a single int array for broadcast
    vector<int> flat;
    if (rank == 0) {
        flat.reserve(N * n);
        for (auto &p : perms)
            flat.insert(flat.end(), p.begin(), p.end());
    } else {
        flat.resize(N * n);
    }
    MPI_Bcast(flat.data(), N*n, MPI_INT, 0, MPI_COMM_WORLD);

    // reconstruct perms on non-root ranks
    if (rank != 0) {
        perms.resize(N, vector<int>(n));
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < n; ++j)
                perms[i][j] = flat[i*n + j];
    }

    // build an index map from permutation → its ordinal
    unordered_map<string,int> idx;
    idx.reserve(N);
    for (int i = 0; i < N; ++i) {
        string key;
        key.reserve(n);
        for (int x : perms[i]) key.push_back(char('0'+x));
        idx[key] = i;
    }
    vector<int> root_perm = perms[0];

    // prepare per‐rank slice bounds
    auto slice_bounds = [&](int rank)->pair<int,int>{
        int per = (N-1 + size -1)/size;
        int start = 1 + rank*per;
        int end   = min(N, 1 + (rank+1)*per);
        return { start, end };
    };

    // for each IST T_t
    for (int t = 1; t <= n-1; ++t) {
        vector<int> parent_local;
        auto [lo, hi] = slice_bounds(rank);
        int slice_size = hi - lo;
        parent_local.resize(slice_size);

        // hybrid parallel: each thread handles part of [lo…hi)
        #pragma omp parallel for schedule(static)
        for (int idx_i = 0; idx_i < slice_size; ++idx_i) {
            int i = lo + idx_i;
            auto p = Parent1(perms[i], t, root_perm);
            // string‐key lookup
            string sk; sk.reserve(n);
            for (int x: p) sk.push_back(char('0'+x));
            parent_local[idx_i] = idx[sk];
        }

        // gather all slices to root's parent_global
        vector<int> parent_global;
        if (rank == 0) parent_global.resize(N);
        // tell MPI exactly how many ints each rank sends
        vector<int> recvcounts(size), displs(size);
        for (int r = 0; r < size; ++r) {
            auto [rlo, rhi] = slice_bounds(r);
            recvcounts[r] = rhi - rlo;
            displs[r]     = rlo;
        }
        MPI_Gatherv(parent_local.data(), slice_size, MPI_INT,
                    parent_global.data(), recvcounts.data(),
                    displs.data(), MPI_INT, 0, MPI_COMM_WORLD);

        // root does children‐build & BFS print
        if (rank == 0) {
            parent_global[0] = -1;
            vector<vector<int>> children(N);
            for (int i = 1; i < N; ++i)
                children[parent_global[i]].push_back(i);

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
    }

    MPI_Finalize();
    return 0;
}
