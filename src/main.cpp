#include "func_state.h"
#include "global_variables.h"
#include <unistd.h>
#include <cstdlib>
#include <algorithm>

#ifndef PARAM_TIME_LIMIT
#define PARAM_TIME_LIMIT 3600.0
#endif

char *Instance_name;	//instance name
vector<vector<int>> adj;   // �ڽӱ�

vector<int> Degree;         //degree of each vertex
int Num_v;				//number of vertices
long long Num_e;              //number of edges
double Density;         //density of graph
int K_opt;              //optimal size of IUC (available for some instances

std::vector<int> Must_in_S;      // �Ƿ����ѡ
std::vector<int> Must_not_in_S;  // �Ƿ��ֹѡ

std::set<std::pair<int, int>> forbidden_diff;  // �洢�������Ľڵ��
std::unordered_map<int, std::unordered_set<int>> remove_at_least_one;  // �洢ÿ���ڵ���Ҫɾ�����ھ�

int* degree;
std::vector<int> ordering;
std::unordered_map<int, int> core_number;  // �洢ÿ���ڵ�ĺ�����
int Compo_cnt;  // ��ͨ��������

double Time_limit, Start_time, Run_time;

int best_size;
std::vector<int> best_S;

int* group_of;
std::vector<std::vector<int>> groups;
int num_groups;

std::vector<CGColumn> columns;

int sum_it;
int Column_MWIS_RLS_Runs = 0;
int Column_MWIS_RLS_Used = 0;
int Column_MWIS_RLS_Improved = 0;
int Column_MWIS_RLS_Selected_Columns = 0;
int Column_MWIS_RLS_Weight = 0;
int Column_MWIS_RLS_Input_Columns = 0;
int Column_MWIS_RLS_Exit_Code = -1;
int Column_MWIS_RLS_Timed_Out = 0;
int Column_MWIS_RLS_Input_Max_Weight = 0;
int Column_MWIS_RLS_Incumbent_Components = 0;
int Column_MWIS_RLS_Incumbent_Columns = 0;
int Column_MWIS_RLS_Incumbent_Weight = 0;
int Column_MWIS_RLS_Incumbent_Superset_Components = 0;
int Column_MWIS_RLS_Incumbent_Superset_Columns = 0;
int Column_MWIS_RLS_Incumbent_Superset_Vertices = 0;
int Column_MWIS_RLS_Solver_Selected_Columns = 0;
int Column_MWIS_RLS_Incumbent_Candidate_Columns = 0;
int Column_MWIS_RLS_Incumbent_Candidate_Used = 0;
int Column_MWIS_RLS_Repaired_Size = 0;
double Column_MWIS_RLS_Time = 0.0;
double Column_MWIS_Build_Time = 0.0;
long long Column_MWIS_Conflict_Edges = 0;

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		printf("show usage: maximum_iuc.exe input_file optimal_k");
		exit(-1);
	}
	Instance_name = argv[1];
	K_opt = atoi(argv[2]);

	unsigned seed = (unsigned)time(NULL) ^ ((unsigned)getpid() << 16);
	if (argc >= 4) {
		seed = (unsigned)strtoul(argv[3], nullptr, 10);
	}
	srand(seed);

	//parameters 
	Time_limit = PARAM_TIME_LIMIT;
	Compo_cnt = 0;

	Start_time = clock();

	read_instance(); 
	allocate_memory();

	Start_time = clock(); 
	if (!(K_opt > 0 && best_size >= K_opt)) {
		run_tabu_search_multi();
	}

	long long cnt_in = 0, cnt_out = 0;
	for (int i = 0; i < Num_v; ++i) {
		cnt_in += (Must_in_S[i] != 0);
		cnt_out += (Must_not_in_S[i] != 0);
	}

	for (const CGColumn& col : columns) {
		if ((int)col.vertices.size() > best_size) {
			best_size = (int)col.vertices.size();
			best_S = col.vertices;
			Compo_cnt = 1;
			Run_time = (double)(clock() - Start_time) / CLOCKS_PER_SEC;
		}
	}

	int comp_cnt = 0;
	bool feasible = validate_iuc(best_S, comp_cnt);
	if (feasible) {
		//std::cout << "Final solution is a valid IUC.\n";
		Compo_cnt = comp_cnt;
	}
	else {
		std::cout << "final best_S solution is NOT a valid IUC.\n";
		exit(-1);
	}

	int cg_col_cnt = (int)columns.size();
	int max_clique_size_in_columns = 0;

	for (const CGColumn& col : columns) {
		int sz = (int)col.vertices.size();
		if (sz > max_clique_size_in_columns) {
			max_clique_size_in_columns = sz;
		}
	}



	cout << Instance_name << " " << Num_v << " " << Num_e << " " ;
	std::cout << "Density: " << Density << "  -----";
	std::cout << "K_opt: " << K_opt << "  -----";
	std::cout << "Best solution size = " << best_size << " ";
	std::cout << "Run_time: " << Run_time << "  -----";
	std::cout << "Compo_cnt: " << Compo_cnt << "  -----";
	std::cout << "Must_in_S: " << cnt_in << "  -----";
	std::cout << "Must_not_in_S: " << cnt_out << "  -----";
	std::cout << "Vertices: ";

	int cnt = 0;
	for (int v : best_S) {
		if (cnt) cout << ",";
		cnt++;
		std::cout << v;
	}
	std::cout << std::endl;
	

	return 0;
}
