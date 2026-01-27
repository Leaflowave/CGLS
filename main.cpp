#include "func_state.h"
#include "global_variables.h"
#include <unistd.h>

char *Instance_name;	//instance name
vector<vector<int>> adj;   // 邻接表

vector<int> Degree;         //degree of each vertex
int Num_v;				//number of vertices
int Num_e;              //number of edges
double Density;         //density of graph
int K_opt;              //optimal size of IUC (available for some instances

std::vector<int> Must_in_S;      // 是否必须选
std::vector<int> Must_not_in_S;  // 是否禁止选

std::set<std::pair<int, int>> forbidden_diff;  // 存储不允许的节点对
std::unordered_map<int, std::unordered_set<int>> remove_at_least_one;  // 存储每个节点需要删除的邻居

int* degree;
std::vector<int> ordering;
std::unordered_map<int, int> core_number;  // 存储每个节点的核心数
int Compo_cnt;  // 连通分量数量

double Time_limit, Start_time, Run_time;

int best_size;
std::vector<int> best_S;

int* group_of;
std::vector<std::vector<int>> groups;
int num_groups;


int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		printf("show usage: maximum_iuc.exe input_file optimal_k");
		exit(-1);
	}
	Instance_name = argv[1];
	K_opt = atoi(argv[2]);

	srand(unsigned(time(NULL)));

	//parameters 
	Time_limit = 3600.0;
	Compo_cnt = 0;

	read_instance(); 
	allocate_memory();

	Start_time = clock(); 
	
	run_tabu_search_multi();


	long long cnt_in = 0, cnt_out = 0;
	for (int i = 0; i < Num_v; ++i) {
		cnt_in += (Must_in_S[i] != 0);
		cnt_out += (Must_not_in_S[i] != 0);
	}

	int comp_cnt = 0;
	bool feasible = validate_iuc(best_S, comp_cnt);
	if (feasible) {
		//std::cout << "Final solution is a valid IUC.\n";
	}
	else {
		std::cout << "final best_S solution is NOT a valid IUC.\n";
		exit(-1);
	}

	cout << Instance_name << " "<<Num_v << " " << Num_e<<" ";
	std::cout << "Density: " << Density << "  -----";
	std::cout << "K_opt: " << K_opt << "  -----";
	std::cout << "Best solution size = " << best_size << " ";
	std::cout << "Run_time: " << Run_time <<"  -----";
	std::cout << "Compo_cnt: " << Compo_cnt << "  -----";
	std::cout << "Must_in_S: " << cnt_in << "  -----";
	std::cout << "Must_not_in_S: " << cnt_out << "  -----";
	std::cout << "Vertices: ";
	int cnt = 0;
	for (int v : best_S) {
		if (cnt)cout << ",";
		cnt++;
		std::cout << v ;
	}
	std::cout << std::endl;
	

	return 0;
}
//for (int i = 0; i < Num_v; ++i) {
	//	if (Must_in_S[i] != 0) {
	//		cout << i << " ";
	//	}
	//}
	//cout << endl;

	//cout << "-----------------" << endl;

	//for (int i = 0; i < Num_v; ++i) {
	//	if (Must_not_in_S[i] != 0) {
	//		cout << i << " ";
	//	}
	//}
	//cout << endl;
