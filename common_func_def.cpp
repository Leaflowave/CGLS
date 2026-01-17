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
	int max_edg = 0;

	vector<pair<int, int>> vp;

	while (!FIC.eof())
	{
		char bidon[100];
		if (strcmp(StrReading, "p") == 0)
		{
			FIC >> bidon >> Num_v >> Num_e;
			Density = 2.0 * Num_e / (Num_v * (Num_v - 1));
		
			Degree = new int[Num_v];
			memset(Degree, 0, sizeof(int) * Num_v);


		}
		if (strcmp(StrReading, "e") == 0)
		{
			int x1, x2;
			FIC >> x1 >> x2;
			x1--; x2--;

			vp.push_back({ x1,x2 });
			vp.push_back({ x2,x1 });

			if (x1 < 0 || x2 < 0 || x1 >= Num_v || x2 >= Num_v)
			{
				printf("### Error of node x1: %d, x2: %d ###\n", x1, x2);
				exit(-1);
			}
	
			Degree[x1]++;
			Degree[x2]++;
			max_edg++;
		}
		FIC >> StrReading;
	}

	sort(vp.begin(), vp.end());

	edges = new int[Num_e * 2];
	memset(edges, 0, sizeof(int) * Num_e * 2);

	pstart = new int[Num_v + 1];
	pstart[0] = 0;
	int idx = 0;
	for (int i = 0; i < Num_v; ++i)
	{
		pstart[i + 1] = pstart[i];
		while (idx < vp.size() && vp[idx].first == i)
		{
			edges[pstart[i + 1]++] = vp[idx++].second;
		}
	}

		
	if (max_edg != Num_e)
	{
		printf("### Error max_edge != nb_edge, Num_e: %d, max_edge: %d ###\n", Num_e, max_edg);
		exit(-1);
	}

	//build_complement_graph_csr();//MPC

	FIC.close();

#ifdef DEBUG
	for (int i = 0; i < vp.size(); i++) {
		if (i > 0 && vp[i].first == vp[i - 1].first && vp[i].second == vp[i - 1].second) {
			printf("WA in read graph\n");
			exit(-1);
		}
	}

	/*for (int i = 0; i < Num_v; ++i)
	{
		for (int j = pstart[i]; j < pstart[i + 1]; ++j) {
			if (Edge[i][edges[j]] == 0) {
				cout << "error in Edge" << endl;
				exit(-1);
			}
		}
	}*/

	//TODO MPC: The MPC number was found by computing the IUC number of the complement graph
	printf("running MPC\n");
	for (int i = 0; i < Num_v; i++)
	{
		for (int j = 0; j < Num_v; j++)
		{
			if (Edge[i][j] > 0)
				Edge[i][j] = 0;
			else
				Edge[i][j] = 1;

			if (i == j)
				Edge[i][j] = 0;
		}
	}

	for (int i = 0; i < Num_v; i++)
	{
		int cnt = 0;
		for (int j = 0; j < Num_v; j++)
		{
			if (Edge[i][j] > 0) {
				cnt++;
			}
		}
		if (cnt != Degree[i]) {
			cout << cnt << " " << Degree[i] << " error in degree" << endl;
			exit(-1);
		}
	}
//	Num_e = (double) Num_v * (Num_v - 1) / 2.0 - Num_e;
#endif

	/*printf("running IUC\n");
	printf("Instance_name: %s, Num_v: %d, Num_e: %d, Density: %.6f\n\n",
			Instance_name, Num_v, Num_e, Density);*/

#ifdef DEBUG
	printf("Num_v: %d, Num_e: %d\n", Num_v, Num_e);
	for (int i = 0; i < Num_v; i++)
		for (int j = 0; j < Num_v; j++)
			if (Edge[i][j] > 0)
				printf("e %d %d\n", i + 1, j + 1);
#endif
}

void build_complement_graph_csr()
{
	int n = Num_v;
	long long m_orig = Num_e;  // 原图无向边数
	long long m_comp = 1LL * n * (n - 1) / 2 - m_orig;
	if (m_comp < 0) {
		printf("### Error: complement edge number < 0 ###\n");
		exit(-1);
	}

	// 补图每个点的度：degC(u) = (n-1) - deg(u)
	int* degC = new int[n];
	long long sumDegC = 0; // 补图“有向边”总数 
	for (int u = 0; u < n; ++u) {
		int deg = pstart[u + 1] - pstart[u]; // 原图 CSR 度
		degC[u] = (n - 1) - deg;
		if (degC[u] < 0) {
			printf("### Error: degC[%d] < 0 (n=%d, deg=%d) ###\n", u, n, deg);
			exit(-1);
		}
		sumDegC += degC[u];
	}

	if (sumDegC != 2LL * m_comp) {
		printf("### Warning: sumDegC(%lld) != 2*m_comp(%lld). input may have duplicates. ###\n",
			sumDegC, 2LL * m_comp);
		exit(-1);
	}

	// 分配补图 CSR
	int* pstartC = new int[n + 1];
	pstartC[0] = 0;
	for (int u = 0; u < n; ++u) pstartC[u + 1] = pstartC[u] + degC[u];

	int* edgesC = new int[(size_t)sumDegC];

	for (int u = 0; u < n; ++u) {
		int write = pstartC[u];
		int idx = pstart[u];
		int end = pstart[u + 1];

		for (int v = 0; v < n; ++v) {
			if (v == u) continue;

			while (idx < end && edges[idx] < v) idx++;

			if (idx < end && edges[idx] == v) continue;

			edgesC[write++] = v;
		}

		
		if (write != pstartC[u + 1]) {
			printf("### Error: complement CSR fill mismatch at u=%d, write=%d, expect=%d ###\n",
				u, write, pstartC[u + 1]);
			exit(-1);
		}
	}

	delete[] edges;
	delete[] pstart;
	delete[] Degree;

	edges = edgesC;
	pstart = pstartC;
	Degree = degC;

	Num_e = (int)m_comp; 
	Density = 2.0 * Num_e / (1.0 * n * (n - 1));
}


void allocate_memory()
{
	Must_in_S.assign(Num_v, 0);
	Must_not_in_S.assign(Num_v, 0);
}