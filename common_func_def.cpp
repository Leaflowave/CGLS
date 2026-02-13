#include"func_state.h"
#include"global_variables.h"
#include <vector>
#include <queue>
#include <unordered_set>
#include <stack>
#include <algorithm>
#include <string>
#include <sstream>
#include <time.h>
#include <climits>

//read instance
void read_instance()
{
    ifstream FIC;
    FIC.open(Instance_name);
    if (FIC.fail())
    {
        printf("### Error open, Instance_name: %s ###\n", Instance_name);
        exit(-1);
    }

    char StrReading[MAXCAN];
    FIC >> StrReading;

    long long max_edg = 0;
    vector<pair<int, int>> vp;   
    vp.reserve(1024);

    Num_v = 0;
    Num_e = 0;

    while (!FIC.eof())
    {
        char bidon[100];

        if (strcmp(StrReading, "p") == 0)
        {
            FIC >> bidon >> Num_v >> Num_e;
            Density = 2.0 * Num_e / (Num_v * (Num_v - 1));

            Degree.assign(Num_v, 0);
            adj.clear();
            adj.resize(Num_v);
        }
        else if (strcmp(StrReading, "e") == 0)
        {
            int x1, x2;
            FIC >> x1 >> x2;
            x1--; x2--;

            if (x1 < 0 || x2 < 0 || x1 >= Num_v || x2 >= Num_v)
            {
                printf("### Error of node x1: %d, x2: %d ###\n", x1, x2);
                exit(-1);
            }

            vp.push_back({ x1, x2 });
            vp.push_back({ x2, x1 });

            Degree[x1]++;
            Degree[x2]++;
            max_edg++;
        }

        FIC >> StrReading;
    }

    if (max_edg != Num_e)
    {
        printf("### Error max_edge != nb_edge, Num_e: %lld, max_edge: %lld ###\n",
            Num_e, max_edg);
        exit(-1);
    }

    sort(vp.begin(), vp.end());

    for (int i = 0; i < Num_v; ++i)
        adj[i].reserve(Degree[i]);

    int idx = 0;
    for (int i = 0; i < Num_v; ++i)
    {
        while (idx < (int)vp.size() && vp[idx].first == i)
        {
            if (!adj[i].empty() && adj[i].back() == vp[idx].second) {

                idx++;
                continue;
            }
            adj[i].push_back(vp[idx].second);
            idx++;
        }
    }

    //build_complement_graph_csr();

    FIC.close();

#ifdef DEBUG
    for (int i = 1; i < (int)vp.size(); i++) {
        if (vp[i].first == vp[i - 1].first && vp[i].second == vp[i - 1].second) {
            printf("WA in read graph: duplicate edge in input\n");
            exit(-1);
        }
    }

    for (int i = 0; i < Num_v; i++) {
        if ((int)adj[i].size() != Degree[i]) {
            printf("degree mismatch at %d: adj=%d Degree=%d\n",
                i, (int)adj[i].size(), Degree[i]);
            exit(-1);
        }
    }
#endif
}


void build_complement_graph_csr()
{
    int n = Num_v;

    static vector<int> mark;
    static int stamp = 1;
    mark.assign(n, 0);

    vector<vector<int>> adjC(n);

    long long sumDegC = 0; 
    for (int u = 0; u < n; ++u) {

        for (int w : adj[u]) mark[w] = stamp;

        adjC[u].reserve(n - 1 - (int)adj[u].size());
        for (int v = 0; v < n; ++v) {
            if (v == u) continue;            
            if (mark[v] != stamp) adjC[u].push_back(v);
        }

        sumDegC += (long long)adjC[u].size();

        ++stamp;
        if (stamp == INT_MAX) { 
            std::fill(mark.begin(), mark.end(), 0);
            stamp = 1;
        }
    }

    adj.swap(adjC);

    Degree.assign(n, 0);
    for (int u = 0; u < n; ++u) Degree[u] = (int)adj[u].size();

    if (sumDegC % 2 != 0) {
        printf("### Warning: complement sumDegC is odd (%lld). Graph may be directed / not symmetric. ###\n", sumDegC);
        exit(-1);
    }

    Num_e = (long long)(sumDegC / 2);
    Density = 2.0 * Num_e / (1.0 * n * (n - 1));
}


void allocate_memory()
{
	Must_in_S.assign(Num_v, 0);
	Must_not_in_S.assign(Num_v, 0);
}