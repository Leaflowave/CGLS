#include <vector>
#include <queue>
#include <unordered_set>
#include <set>
#include <map>
#include <string>
#ifndef _FUNC_STATE_H
#define _FUNC_STATE_H

void read_instance();
void allocate_memory();
void build_complement_graph_csr();


// Compute connected components of G[S_nodes] 
void iuc_components(const std::vector<int>& S_nodes,
	std::vector<std::vector<int>>& comps);


// Randomly expand S_init into a maximal IUC using the global graph.
std::vector<int> random_maximal_iuc(const std::vector<int>& S_init = std::vector<int>(),
	const std::vector<double>* weights = nullptr);


bool validate_iuc(const std::vector<int>& S_nodes, int& cnt);


// ===== Tabu search for IUC  =====
void tabu_search_core_conflict(const std::vector<int>& init_S_vec,
	int& cur_best,
	std::vector<int>& best_S_vec,
	std::vector<int>& freq);

void run_tabu_search_multi();

bool hasEdge(int u, int v);

bool is_clique_comp_fast(const std::vector<int>& comp);

void reduction(int k);

void reduction();

void dfs_comp(int v, int comp_num, std::vector<char>& visited, std::vector<int>& component);

void reduction_rule_1();

void reduction_rule_2();

void reduction_rule_3(int k);

void neighbors_of(int u, std::vector<int>& nei);

void degeneracy_ordering();

int scr_upperbound_color(const std::vector<int>& nodes, const std::vector<int>& ordering);

void reduction_rule_4(int k);

void second_neighbors_of(int u, const std::vector<int>& N1, std::vector<int>& N2);

int greedy_maximal_matching_bipartite(const std::vector<int>& N1, const std::vector<int>& N2);

void reduction_rule_5(int k);

std::unordered_set<int> get_second_neighbors(int u);

void dfs_neig(int node, const std::unordered_set<int>& neighbors, std::vector<bool>& visited, std::vector<int>& component);

bool has_at_least_two_clique_components(const std::vector<int>& S_nodes);

void reduction_rule_6();

void closed_neighborhood_set(int v, std::set<int>& sig);

int build_critical_cliques_groups_sets(int*& group_of,
	std::vector<std::vector<int>>& groups,
	std::vector<int>& group_sizes);

void build_contracted_graph_sets(const int* group_of,
	int num_groups,
	int**& new_Edge);

void apply_rule8_sets(const int* group_of,
	const std::vector<int>& group_sizes,
	int num_groups,
	int k,
	bool*& candidate_in);

void reduction_rule_7_8_sets(int k,
	int*& group_of,
	std::vector<std::vector<int>>& groups,
	int& num_groups);




#endif
