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
#include <cstdint>

void iuc_components(const std::vector<int>& S_nodes,
	std::vector<std::vector<int>>& comps) {
	comps.clear();
	if (S_nodes.empty()) return;

	static std::vector<uint32_t> inS;
	static std::vector<uint32_t> visited;
	static uint32_t stamp = 1;

	if ((int)inS.size() != Num_v) {
		inS.assign(Num_v, 0);
		visited.assign(Num_v, 0);
		stamp = 1;
	}

	++stamp;
	if (stamp == 0) { 
		std::fill(inS.begin(), inS.end(), 0);
		std::fill(visited.begin(), visited.end(), 0);
		stamp = 1;
	}

	for (int v : S_nodes) {
		if (v >= 0 && v < Num_v) inS[v] = stamp;
	}

	std::vector<int> st;
	st.reserve(1024);

	for (int start : S_nodes) {
		if (start < 0 || start >= Num_v) continue;
		if (inS[start] != stamp) continue;
		if (visited[start] == stamp) continue;

		std::vector<int> comp;
		st.clear();
		st.push_back(start);
		visited[start] = stamp;

		while (!st.empty()) {
			int v = st.back();
			st.pop_back();
			comp.push_back(v);

			for (int u : adj[v]) {
				if (inS[u] != stamp) continue;
				if (visited[u] == stamp) continue;
				visited[u] = stamp;
				st.push_back(u);
			}
		}

		comps.push_back(std::move(comp));
	}
}


std::vector<int> random_maximal_iuc(const std::vector<int>& S_init,
	const std::vector<double>* weights) {

	// Deduplicate S_init and build membership map.
	std::vector<char> inS(Num_v, 0);
	std::vector<int> S;
	S.reserve(S_init.size());
	for (int v : S_init) {
		if (v < 0 || v >= Num_v) {
			cout << "error in random_S_init" << endl;
			exit(-1);
			continue;
		}
		if (!inS[v]) {
			inS[v] = 1;
			S.push_back(v);
		}
	}

	std::vector<std::vector<int>> comps;
	iuc_components(S, comps);

	std::vector<int> comp_index(Num_v, -1);
	for (int ci = 0; ci < (int)comps.size(); ++ci) {
		for (int v : comps[ci]) {
			if (v >= 0 && v < Num_v)
				comp_index[v] = ci;
			else {
				cout << "error in comp_index" << endl;
				exit(-1);
			}
		}
	}

	std::vector<int> comp_size(comps.size(), 0);
	for (int ci = 0; ci < (int)comps.size(); ++ci) comp_size[ci] = (int)comps[ci].size();

	std::vector<int> cover_cnt(Num_v, 0);
	std::vector<int> touch_comp(Num_v, -1);
	std::vector<int> touch_cnt(Num_v, 0);
	std::vector<uint8_t> touch_multi(Num_v, 0);


	std::vector<double> base_w(Num_v, 1.0);
	if (weights != nullptr && !weights->empty()) {
		int WN = (int)weights->size();
		for (int v = 0; v < Num_v; ++v) {
			double x = (v < WN ? (*weights)[v] : 0.0);
			base_w[v] = std::exp(-x);
		}
	}

	std::vector<int> uncovered;
	std::vector<int> pos_uncovered(Num_v, -1);

	auto remove_from_uncovered = [&](int v) {
		int pos = pos_uncovered[v];
		if (pos < 0) return;
		int last = uncovered.back();
		uncovered[pos] = last;
		pos_uncovered[last] = pos;
		uncovered.pop_back();
		pos_uncovered[v] = -1;
		};

	auto add_to_uncovered = [&](int v) {
		if (pos_uncovered[v] != -1) return;
		pos_uncovered[v] = (int)uncovered.size();
		uncovered.push_back(v);
		};

	auto is_joinable = [&](int v) -> bool {
		if (v < 0 || v >= Num_v) return false;
		if (Must_not_in_S[v]) return false;
		if (inS[v]) return false;
		if (touch_multi[v]) return false;
		int cid = touch_comp[v];
		if (cid < 0 || cid >= (int)comp_size.size()) return false;
		return touch_cnt[v] == comp_size[cid];
		};


	std::vector<int> pool;                 
	std::vector<int> pos_pool(Num_v, -1);  
	std::vector<int> pool_cid(Num_v, -2); 
	double pool_sum_w = 0.0;              

	auto pool_add = [&](int v, int cid) {
		if (pos_pool[v] != -1) {
			pool_cid[v] = cid;
			return;
		}
		pos_pool[v] = (int)pool.size();
		pool.push_back(v);
		pool_cid[v] = cid;
		pool_sum_w += base_w[v];
		};

	auto pool_remove = [&](int v) {
		int pos = pos_pool[v];
		if (pos < 0) return;
		pool_sum_w -= base_w[v];
		int last = pool.back();
		pool[pos] = last;
		pos_pool[last] = pos;
		pool.pop_back();
		pos_pool[v] = -1;
		pool_cid[v] = -2;
		};

	auto refresh_candidate = [&](int v) {
		if (v < 0 || v >= Num_v) return;
		if (Must_not_in_S[v] || inS[v]) { pool_remove(v); return; }


		if (cover_cnt[v] == 0) {
			pool_add(v, -1);
			return;
		}


		if (is_joinable(v)) {
			pool_add(v, touch_comp[v]);
			return;
		}

		pool_remove(v);
		};


	for (int s : S) {
		int cid = comp_index[s];
		for (int x : adj[s]) {
			if (x < 0 || x >= Num_v) continue;
			if (Must_not_in_S[x]) continue;
			if (inS[x]) continue;

			cover_cnt[x]++;

			if (touch_multi[x]) continue;
			if (touch_comp[x] == -1) {
				touch_comp[x] = cid;
				touch_cnt[x] = 1;
			}
			else if (touch_comp[x] == cid) {
				touch_cnt[x]++;
			}
			else {
				touch_multi[x] = 1;
			}
		}
	}


	uncovered.reserve(Num_v);
	pool.reserve(Num_v);

	for (int v = 0; v < Num_v; ++v) {
		if (Must_not_in_S[v] || inS[v]) continue;

		if (cover_cnt[v] == 0) add_to_uncovered(v);

		refresh_candidate(v);
	}

	auto add_vertex_to_S = [&](int v, int cid) {
		if (inS[v]) {
			cout << "error in add" << endl;
			exit(-1);
		}

		inS[v] = 1;
		S.push_back(v);

		remove_from_uncovered(v);
		pool_remove(v);

		if (cid == -1) {
			cid = (int)comp_size.size();
			comp_size.push_back(1);
			comps.push_back(std::vector<int>(1, v));
			comp_index[v] = cid;
		}
		else {
			comp_size[cid] += 1;
			comps[cid].push_back(v);
			comp_index[v] = cid;
		}

		for (int x : adj[v]) {
			if (x < 0 || x >= Num_v) continue;
			if (Must_not_in_S[x]) continue;
			if (inS[x]) continue;

			if (cover_cnt[x] == 0) remove_from_uncovered(x);
			cover_cnt[x]++;

			if (!touch_multi[x]) {
				if (touch_comp[x] == -1) {
					touch_comp[x] = cid;
					touch_cnt[x] = 1;
				}
				else if (touch_comp[x] == cid) {
					touch_cnt[x]++;
				}
				else {
					touch_multi[x] = 1;
				}
			}

			refresh_candidate(x);
		}
		};


	auto pick_from_pool = [&]() -> int {
		if (pool.empty()) return -1;
		if (weights == nullptr || weights->empty() || pool_sum_w <= 0.0) {
			int idx = std::rand() % pool.size();
			return pool[idx];
		}

		double r = (double)std::rand() / (double)RAND_MAX * pool_sum_w;
		double acc = 0.0;
		for (int i = 0; i < (int)pool.size(); ++i) {
			int v = pool[i];
			acc += base_w[v];
			if (r <= acc) return v;
		}
		return pool.back(); 
		};

	while (true) {
		if (pool.empty()) break;

		int v = pick_from_pool();
		if (v < 0) break;

		int cid = pool_cid[v];

		if (cid == -1) {
			if (Must_not_in_S[v] || inS[v] || cover_cnt[v] != 0) {
				refresh_candidate(v);
				continue;
			}
			add_vertex_to_S(v, -1);
		}
		else {
			if (!is_joinable(v)) {
				refresh_candidate(v);
				continue;
			}
			add_vertex_to_S(v, cid);
		}


		if ((double)(clock() - Start_time) / CLOCKS_PER_SEC >= Time_limit) {
			break;
		}

	}

	if ((int)S.size() > best_size) {
		best_size = (int)S.size();
		best_S = S;
		Run_time = (double)(clock() - Start_time) / CLOCKS_PER_SEC;
	}

	return S;
}




static inline void get_neighbors(int v, vector<int>& nei) {
	nei.clear();
	for (int u : adj[v]) {
		nei.push_back(u);
	}
}

// count |S ∩ N(v)|
static int count_neighbors_in_set(int v, const unordered_set<int>& S) {
	int cnt = 0;
	for (int u : adj[v]) {
		if (S.count(u)) ++cnt;
	}
	return cnt;
}

static std::unordered_set<int> get_nei(int v) {
	std::unordered_set<int> nei;
	for (int u : adj[v]) {
		nei.insert(u);
	}
	return nei;
}

// |cc ∩ N(v)|
static int count_neighbors_in_comp(int v, const unordered_set<int>& cc) {
	int cnt = 0;
	for (int u : adj[v]) {
		if (cc.count(u)) ++cnt;
	}
	return cnt;
}


static inline std::unordered_set<int> neighbors_of(int v) {
	std::unordered_set<int> nei;
	for (int u : adj[v]) {
		nei.insert(u);
	}
	return nei;
}


struct VertexInfo {
	int nbr_in_S = 0;                         // |N(i) ∩ S|
	std::unordered_map<int, int> nbr_in_C;    // cid -> |N(i) ∩ C|
	int conflict = -1;                        
	int min_conflict_cluster = -1;           
	int judge_confict = 0;
};

std::vector<VertexInfo> vertex_info; // size = Num_v
std::vector<std::unordered_set<int>> clusters;  


static void rebuild_from_S_conflict(const std::unordered_set<int>& S)
{
	if ((int)vertex_info.size() != Num_v) {
		cout << "error in vertex_info" << endl;
		exit(-1);
	}


	clusters.clear();
	clusters.reserve(S.size());

	std::vector<int> S_nodes(S.begin(), S.end());
	std::vector<std::vector<int>> comps;
	iuc_components(S_nodes, comps); 

	for (auto& comp : comps) {
		std::unordered_set<int> clique;
		clique.reserve(comp.size() * 2);
		for (int v : comp) clique.insert(v);
		clusters.push_back(std::move(clique));
	}

	// 初始化 |N(v) ∩ S| 和 |N(v) ∩ C|
	for (int v = 0; v < Num_v; ++v) {
		vertex_info[v].nbr_in_S = 0;
		vertex_info[v].nbr_in_C.clear();
	}
	std::vector<int> vertex_to_cid(Num_v, -1);
	for (int cid = 0; cid < (int)clusters.size(); ++cid) {
		for (int u : clusters[cid]) {
			vertex_to_cid[u] = cid;
		}
	}

	for (const int u : S) {
		const int cid = vertex_to_cid[u];

		for (int v : adj[u]) {

			vertex_info[v].nbr_in_S++;

			if (cid != -1) {
				vertex_info[v].nbr_in_C[cid]++;
			}
		}
	}


	for (int v = 0; v < Num_v; ++v) {
		vertex_info[v].judge_confict = 0;
		if (S.count(v)) continue;  

		bool flag = false;//判断是否是PA / IS
		bool st = false;//判断是否是OM / OA
		int nbrS = vertex_info[v].nbr_in_S;
		int best_conflict = nbrS;
		int best_cid = -1;
		int judge_conf = 0;

		for (auto& p : vertex_info[v].nbr_in_C) {
			int cid = p.first;
			int val = p.second;
			if (cid < 0 || cid >= (int)clusters.size()) continue; 

			int csize = (int)clusters[cid].size();
			int cost_joinC = (csize - val) + (nbrS - val);
			if (val > csize) {
				cout << "error in val" << endl;
				exit(-1);
			}

			bool st = false;

			if (cost_joinC < best_conflict) {
				best_conflict = cost_joinC;
				best_cid = cid;

				if (cost_joinC == 0) {
					if (nbrS == 0 || csize == val) {

					}
					else {
						cout << csize << " " << val << " " << nbrS << "  error in 0000000 _ rebuild_from_S_conflict" << endl;
						exit(-1);
					}
				}
				else if (cost_joinC == 1) {
					//OM
					if (nbrS == csize - 1 && csize > 1 && val == csize - 1) {
						judge_conf = 2;
					}
					// OA
					if (nbrS == csize + 1 && val == csize) {
						judge_conf = 3;
					}
				}

			}

		}

		vertex_info[v].conflict = best_conflict;
		vertex_info[v].min_conflict_cluster = best_cid;
		vertex_info[v].judge_confict = judge_conf;

	}
}


void update_add_conflict(int v, int cid, std::unordered_set<int>& S)
{
	S.insert(v);

	if (cid == (int)clusters.size()) {
		clusters.push_back({ v }); 
	}
	else {
		if (cid < 0 || cid >= (int)clusters.size()) {
			std::cerr << "[Error] update_add_conflict invalid cid=" << cid
				<< " clusters.size=" << clusters.size() << std::endl;
			abort();
		}
		if (clusters[cid].count(v)) {
			std::cerr << "[Error] vertex " << v << " already in cluster " << cid << std::endl;
			abort();
		}
		clusters[cid].insert(v);
	}


	for (int u : adj[v]) {

		vertex_info[u].nbr_in_S += 1;

		auto it = vertex_info[u].nbr_in_C.find(cid);
		if (it != vertex_info[u].nbr_in_C.end())
			it->second += 1;
		else
			vertex_info[u].nbr_in_C[cid] = 1;
	}

	for (int u = 0; u < Num_v; ++u) {  
		if (S.count(u)) continue;

		int nbrS = vertex_info[u].nbr_in_S;
		int best_conflict = nbrS;
		int best_cid = -1;
		int judge_conf = 0;

		for (auto& p : vertex_info[u].nbr_in_C) {
			int cid2 = p.first;
			int val = p.second;

			if (cid2 < 0 || cid2 >= (int)clusters.size()) continue;

			int csize = (int)clusters[cid2].size();
			int cost_joinC = (csize - val) + (nbrS - val);

			if (val > csize) {
				cout << "error in val" << endl;
				exit(-1);
			}

			if (cost_joinC < best_conflict) {
				best_conflict = cost_joinC;
				best_cid = cid2;
				if (cost_joinC == 0) {
					if (nbrS == 0 || csize == val) {

					}
					else {
						cout << csize << " " << val << " " << nbrS << "  error in 0000000 _ rebuild_from_S_conflict" << endl;
						exit(-1);
					}
				}
				else if (cost_joinC == 1) {
					//OM
					if (nbrS == csize - 1 && csize > 1 && val == csize - 1) {
						judge_conf = 2;
					}
					// OA
					if (nbrS == csize + 1 && val == csize) {
						judge_conf = 3;
					}
				}
			}

		}

		vertex_info[u].conflict = best_conflict;
		vertex_info[u].min_conflict_cluster = best_cid;
		vertex_info[u].judge_confict = judge_conf;

	}
}


void update_drop_conflict(int v, int cid, std::unordered_set<int>& S)
{

	S.erase(v);
	clusters[cid].erase(v);

	if (clusters[cid].empty()) {
		clusters.erase(clusters.begin() + cid);
		for (auto& info : vertex_info) {
			std::unordered_map<int, int> newmap;
			for (auto& kv : info.nbr_in_C) {
				if (kv.first == cid) continue;
				int new_id = kv.first > cid ? kv.first - 1 : kv.first;
				newmap[new_id] = kv.second;
			}
			info.nbr_in_C.swap(newmap);

			if (info.min_conflict_cluster == cid)
				info.min_conflict_cluster = -1;
			else if (info.min_conflict_cluster > cid)
				info.min_conflict_cluster--;
		}
	}

	for (int u : adj[v]) {

		vertex_info[u].nbr_in_S -= 1;
		if (cid >= 0 && cid < (int)clusters.size()) {
			auto it = vertex_info[u].nbr_in_C.find(cid);
			if (it != vertex_info[u].nbr_in_C.end() && it->second > 0)
				it->second -= 1;
		}
	}

	for (int u = 0; u < Num_v; ++u) {  
		vertex_info[v].judge_confict = 0;
		if (S.count(u)) continue;

		int nbrS = vertex_info[u].nbr_in_S;
		int best_conflict = nbrS;
		int best_cid = -1;
		int judge_conf = 0;

		for (auto& p : vertex_info[u].nbr_in_C) {
			int cid2 = p.first;
			int val = p.second;  
			if (cid2 < 0 || cid2 >= (int)clusters.size()) continue;
			int csize = (int)clusters[cid2].size();
			int cost_joinC = (csize - val) + (nbrS - val);


			if (val > csize) {
				cout << "error in val" << endl;
				exit(-1);
			}

			if (cost_joinC < best_conflict) {
				best_conflict = cost_joinC;
				best_cid = cid2;

				if (cost_joinC == 0) {
					if (nbrS == 0 || csize == val) {

					}
					else {
						cout << csize << " " << val << " " << nbrS << "  error in 0000000 _ rebuild_from_S_conflict" << endl;
						exit(-1);
					}
				}
				else if (cost_joinC == 1) {
					//OM
					if (nbrS == csize - 1 && csize > 1 && val == csize - 1) {
						judge_conf = 2;
					}
					// OA
					if (nbrS == csize + 1 && val == csize) {
						judge_conf = 3;
					}
				}
			}


		}

		vertex_info[u].conflict = best_conflict;
		vertex_info[u].min_conflict_cluster = best_cid;
		vertex_info[u].judge_confict = judge_conf;


	}
}




//  choose one element from a container of ints with weights exp(-freq[v]).
template <typename Container>
static int weighted_choice_exp(const Container& cont, const vector<int>& freq) {

	vector<int> arr;
	arr.reserve(cont.size());
	for (int v : cont) arr.push_back(v);
	if (arr.empty()) return -1;

	vector<double> w(arr.size());
	double sum_w = 0.0;
	for (size_t i = 0; i < arr.size(); ++i) {
		int v = arr[i];
		double wf = std::exp(-(double)freq[v]);
		w[i] = wf;
		sum_w += wf;
	}
	if (sum_w <= 0.0) {

		int idx = std::rand() % arr.size();
		return arr[idx];
	}


	double r = (double)std::rand() / (double)RAND_MAX * sum_w;
	double acc = 0.0;
	for (size_t i = 0; i < arr.size(); ++i) {
		acc += w[i];
		if (r <= acc) return arr[i];
	}
	return arr.back();
}

bool validate_iuc(const std::vector<int>& S_nodes, int& cnt) {
	std::vector<std::vector<int>> comps;
	iuc_components(S_nodes, comps);

	cnt = static_cast<int>(comps.size());

	for (const auto& C : comps) {
		if (!is_clique_comp_fast(C)) {
			return false;
		}
	}
	return true;
}



void tabu_search_core_conflict(const std::vector<int>& init_S_vec,
	int& cur_best,
	std::vector<int>& best_S_vec,
	std::vector<int>& freq)
{

	std::unordered_set<int> S;
	for (int v : init_S_vec) {
		if (v < 0 || v >= Num_v) {
			std::cerr << "[Error] invalid vertex in init_S_vec: " << v << std::endl;
			exit(-1);
		}
		S.insert(v);
	}

	if ((int)freq.size() < Num_v)
		freq.assign(Num_v, 1);

	cur_best = (int)S.size();
	best_S_vec.assign(S.begin(), S.end());

	std::vector<int> tabu_timer(Num_v, 0);


	rebuild_from_S_conflict(S);

	int depth = 0;
	int max_depth = Num_v * 2;

	if (Density > 0.5) {
		max_depth = Num_v / 2;
	}


	while (depth < max_depth)
	{
		if ((double)(clock() - Start_time) / CLOCKS_PER_SEC >= Time_limit) {
			break;
		}
		++depth;
		bool improved = false;

		for (int v = 0; v < Num_v; ++v)
			if (tabu_timer[v] > 0) tabu_timer[v]--;


		std::vector<int> cand_add;
		std::vector<int> cand_IS;
		std::vector<int> cand_PA;

		for (int v = 0; v < Num_v; ++v) {
			if (Must_not_in_S[v])continue;
			if (!S.count(v) && vertex_info[v].conflict == 0) {
				cand_add.push_back(v);
				if (vertex_info[v].nbr_in_S == 0) {
					cand_IS.push_back(v);
				}
				else {
					cand_PA.push_back(v);
				}
			}
		}


		if (!cand_add.empty()) {

			int vin = weighted_choice_exp(cand_add, freq);
			int cid = vertex_info[vin].min_conflict_cluster;
			if (cid < 0) cid = (int)clusters.size(); 

			update_add_conflict(vin, cid, S);
			improved = true;

		}

	
		if (!improved) {
			std::vector<int> cand_swap;
			std::vector<int> cand_OM;
			std::vector<int> cand_OA;

			for (int v = 0; v < Num_v; ++v) {
				if (Must_not_in_S[v])continue;
				if (!S.count(v) && vertex_info[v].conflict == 1 && tabu_timer[v] == 0) {
					cand_swap.push_back(v);
					if (vertex_info[v].judge_confict == 2) {
						cand_OM.push_back(v);
					}
					else if (vertex_info[v].judge_confict == 3) {
						cand_OA.push_back(v);
					}
				}
			}
				

			if (!cand_swap.empty()) {

				int vin;

				if (!cand_OM.empty())
				{
					vin = weighted_choice_exp(cand_OM, freq);
					int cid = vertex_info[vin].min_conflict_cluster;

					if (cid < 0 || cid >= (int)clusters.size()) continue;

					auto& C = clusters[cid];
					int csize = (int)C.size();
					auto it = vertex_info[vin].nbr_in_C.find(cid);
					int in_cc = (it != vertex_info[vin].nbr_in_C.end()) ? it->second : 0;
					int vout = -1;

					// ① OM
					for (int u : C) {
						if (!hasEdge(u, vin) && u != vin) { vout = u; break; }
					}
						

					if (vout >= 0) {
						update_add_conflict(vin, cid, S);
						update_drop_conflict(vout, cid, S);

						int rand_range = std::max(1, (int)cand_swap.size() * 2);
						tabu_timer[vout] = 7 + (std::rand() % rand_range);
						improved = true;
					}

				}
				else if (!cand_OA.empty())
				{
					vin = weighted_choice_exp(cand_OA, freq);
					int cid = vertex_info[vin].min_conflict_cluster;

					if (cid < 0 || cid >= (int)clusters.size()) continue;

					auto& C = clusters[cid];
					int csize = (int)C.size();
					auto it = vertex_info[vin].nbr_in_C.find(cid);
					int in_cc = (it != vertex_info[vin].nbr_in_C.end()) ? it->second : 0;
					int vout = -1;

					// ② OA
					for (int u : adj[vin]) {

						if (S.count(u) && !C.count(u)) { vout = u; break; }
					}

					if (vout >= 0) {
						int cid_out = -1;
						for (int k = 0; k < (int)clusters.size(); ++k)
							if (clusters[k].count(vout)) { cid_out = k; break; }

						if (cid_out >= 0) {
							update_add_conflict(vin, cid, S);
							update_drop_conflict(vout, cid_out, S);

							int rand_range = std::max(1, (int)cand_swap.size() * 2);
							tabu_timer[vout] = 7 + (std::rand() % rand_range);//
							improved = true;
						}
					}



				}


			}
		}

		if ((int)S.size() > cur_best) {
			cur_best = (int)S.size();
			best_S_vec.assign(S.begin(), S.end());
			depth = 0;
		}


		if (depth > (int)S.size() && !improved) {
			std::unordered_set<int> to_remove;
			for (int v : S) {
				if (Must_in_S[v])continue;
				if (Must_not_in_S[v])
				{
					cout << "error in tube" << endl;
					exit(-1);
					to_remove.insert(v);
					continue;
				}
				if ((double)std::rand() / RAND_MAX < 0.4)
					to_remove.insert(v);
			}

			for (int v : to_remove) {
				S.erase(v);
			}


			std::vector<int> S_vec;
			S_vec.reserve(S.size());
			for (int v : S) S_vec.push_back(v);

	
			vector<double> w(Num_v);
			for (int v = 0; v < Num_v; ++v) {
				w[v] = (double)freq[v];
			}
			std::vector<int> S_repaired_vec = random_maximal_iuc(S_vec, &w);


			std::unordered_set<int> S_new;
			S_new.reserve(S_repaired_vec.size() * 2);
			for (int v : S_repaired_vec) S_new.insert(v);
			S.swap(S_new);


			rebuild_from_S_conflict(S);
		
			for (int v : S) freq[v]++;
		}
		else if (!improved) {
			for (int v : S) freq[v]++;
		}
	} // while

}



void run_tabu_search_multi()
{
	degeneracy_ordering();

	reduction();

	std::vector<int> S0;  // 初始解

	for (int i = 0; i < Num_v; ++i) {
		if (Must_in_S[i]) {
			S0.push_back(i);
		}
	}

	std::vector<int> S = random_maximal_iuc(S0, nullptr);

	if (Num_e <= 1000000)
		reduction(Num_v - (int)S.size());

	int comp_cnt = 0;

	bool feasible = validate_iuc(S, comp_cnt);
	if (feasible) {
		//std::cout << "Final solution is a valid IUC.\n";
	}
	else {
		std::cout << "Init solution is NOT a valid IUC.\n";
		exit(-1);
	}
	Compo_cnt = comp_cnt;

	if (K_opt == (int)S.size() || Run_time >= Time_limit) {
		return;
	}

	std::cout << Instance_name << "size of random maximal IUC: " << S.size() << " " << (double)(clock() - Start_time) / CLOCKS_PER_SEC << std::endl;


	// 全局频率数组，顶点 v 被“惩罚”的次数
	vector<int> freq(Num_v, 1);
	int best = 0;//当前所有 epoch 中找到的最好解大小
	vector<int> best_res;//对应的集合
	vector<int> res;//本次 epoch Tabu 搜索结束后的解
	int nonImprove = 0;//最近是否有改进
	bool has_res = false;//标记是否已经有过一次搜索结果

	vertex_info.assign(Num_v, VertexInfo());//

	int it = 0;
	while (true) {
		if ((double)(clock() - Start_time) / CLOCKS_PER_SEC >= Time_limit)break;
		++it;

		// Step: choose initial solution
		vector<int> init_sol;
		if (!has_res || nonImprove > 0) {
			// Build weights from freq
			vector<int> empty_init;
			vector<double> w(Num_v);

			for (int v = 0; v < Num_v; ++v) {
				w[v] = (double)freq[v];
				if (Must_in_S[v]) {
					empty_init.push_back(v);
				}
			}

			init_sol = random_maximal_iuc(empty_init, &w);
			
			if (K_opt == (int)init_sol.size() || Run_time >= Time_limit) {
				return;
			}			

			nonImprove = 1;
		}

		vector<int> copy_init_sol = init_sol;
		nonImprove += 1;


		long long sum_freq = 0;
		for (int x : freq) sum_freq += x;
		if (sum_freq > 5LL * Num_v) {
			for (int v = 0; v < Num_v; ++v) {
				freq[v] = std::max(freq[v] / 2, 1);
			}
		}

		int cur_best = 0;
		res.clear();

		tabu_search_core_conflict(init_sol, cur_best, res, freq);

		has_res = true;


		unordered_set<int> res_set;
		res_set.reserve(res.size() * 2);
		for (int v : res) res_set.insert(v);
		for (int v : copy_init_sol) {
			if (!res_set.count(v)) {

				if (v >= 0 && v < Num_v) {
					freq[v] += 1;
				}
				else {
					cout << "erroe in update freq" << endl;
					exit(-1);
				}
			}
		}

		//cout << "epoch------------" << it << " " << best << " " << (double)(clock() - Start_time) / CLOCKS_PER_SEC << endl;

		if (cur_best > best) {
			Run_time = (double)(clock() - Start_time) / CLOCKS_PER_SEC;

			best = cur_best;
			best_res = res;
			nonImprove = 1;

			int comp_cnt = 0;
			bool feasible = validate_iuc(res, comp_cnt);
			if (feasible) {
				//std::cout << "Final solution is a valid IUC.\n";
			}
			else {
				std::cout << "Final solution is NOT a valid IUC.\n";
				exit(-1);
			}

			Compo_cnt = comp_cnt;

			if (Run_time >= Time_limit || K_opt == best) {
				best_size = best;
				best_S = best_res;
				return;
			}


		}
		else {
			// otherwise
		}
		
	}

	if (best > best_size) {
		best_size = best;
		best_S = best_res;
	}
}



bool hasEdge(int u, int v) {
	if ((int)adj[u].size() > (int)adj[v].size()) {
		std::swap(u, v);
	}

	for (int x : adj[u]) {
		if (x == v) return true;
	}
	return false;

}




bool is_clique_comp_fast(const std::vector<int>& comp) {
	static std::vector<uint32_t> markComp;// 标记是否在当前 comp 内
	static uint32_t stampComp = 1;

	int s = (int)comp.size();
	if (s <= 1) return true;

	if ((int)markComp.size() != Num_v) {
		markComp.assign(Num_v, 0);
		stampComp = 1;
	}

	++stampComp;
	if (stampComp == 0) {
		std::fill(markComp.begin(), markComp.end(), 0);
		stampComp = 1;
	}

	for (int v : comp) markComp[v] = stampComp;

	for (int v : comp) {
		int cnt = 0;
		for (int w : adj[v]) {
			if (markComp[w] == stampComp) ++cnt;
		}
		if (cnt != s - 1) return false;
	}
	return true;
}

void reduction()
{
	reduction_rule_1();
	
	reduction_rule_2();
	
	if (Num_e <= 1000000) {
		reduction_rule_6();
	}
}

void reduction(int k) {
	reduction_rule_3(k);

	reduction_rule_4(k);

	reduction_rule_5(k);

	reduction_rule_7_8_sets(k, group_of, groups, num_groups);
}

// 归约规则 1：检测完全图并返回结果
void reduction_rule_1()
{
	static std::vector<uint32_t> visited;
	static uint32_t vstamp = 1;

	if ((int)visited.size() != Num_v) {
		visited.assign(Num_v, 0);
		vstamp = 1;
	}

	++vstamp;
	if (vstamp == 0) {
		std::fill(visited.begin(), visited.end(), 0);
		vstamp = 1;
	}

	std::vector<int> st;
	std::vector<int> comp;
	st.reserve(1024);
	comp.reserve(1024);

	for (int start = 0; start < Num_v; ++start) {

		if (visited[start] == vstamp) continue;

		st.clear();
		comp.clear();
		st.push_back(start);
		visited[start] = vstamp;

		while (!st.empty()) {
			int v = st.back();
			st.pop_back();
			comp.push_back(v);

			for (int u : adj[v])
			{
				if (visited[u] == vstamp) continue;
				visited[u] = vstamp;
				st.push_back(u);
			}
		}

		if (is_clique_comp_fast(comp)) {
			for (int v : comp) Must_in_S[v] = 1;
		}
	}
}


// 归约规则 2：更新 Must_in_S 集合
void reduction_rule_2() {
	// {x .v. y}
	for (int v = 0; v < Num_v; ++v) {
		if (Must_not_in_S[v]) continue;

		int dv = (int)adj[v].size();
		if (dv != 2) continue;

		int a = adj[v][0];
		int b = adj[v][1];

		int x = -1, y = -1;

		int da = (int)adj[a].size();
		int db = (int)adj[b].size();

		if (!Must_not_in_S[a] && da == 1) { x = a; y = b; }
		else if (!Must_not_in_S[b] && db == 1) { x = b; y = a; }
		else continue;

		if (Must_not_in_S[x]) continue;
		if (Must_in_S[y]) continue;  

		Must_in_S[v] = 1;
		Must_in_S[x] = 1;
		Must_not_in_S[y] = 1;
	}

	// (B) deg(u)=1: IN {u}
	for (int u = 0; u < Num_v; ++u) {
		if (Must_not_in_S[u]) continue;

		int du = (int)adj[u].size();
		if (du != 1) continue;

		Must_in_S[u] = 1;
	}
}


// k + 1个共同邻居
void reduction_rule_3(int k)
{
	forbidden_diff.clear();
	remove_at_least_one.clear();

	std::vector<char> mark(Num_v, 0);

	int test_cnt = 0;

	for (int u = 0; u < Num_v; ++u) {
		if (Degree[u] <= k) continue;

		for (int w : adj[u]) {
			mark[w] = 1;
		}

		for (int v = u + 1; v < Num_v; ++v) {
			if (Degree[v] <= k) continue;

			int common = 0;

			for (int w : adj[v]) {
				if (mark[w]) {
					++common;
					if (common >= k + 1) break;
				}
			}


			if (common >= k + 1) {
				forbidden_diff.insert({ u, v });  

				if (!hasEdge(u, v)) {
					remove_at_least_one[u].insert(v);
					remove_at_least_one[v].insert(u);
					++test_cnt;
				}
			}
		}

		for (int w : adj[u]) {
			mark[w] = 0;
		}


	}

}


// 计算整个图的退化序
void degeneracy_ordering() {
	ordering.clear();

	core_number.clear();
	if (Num_v <= 0) return;

	std::vector<int> deg(Num_v, 0);
	for (int u = 0; u < Num_v; ++u) {
		deg[u] = (int)adj[u].size();
	}


	using PII = std::pair<int, int>;
	std::priority_queue<PII, std::vector<PII>, std::greater<PII>> heap;
	for (int u = 0; u < Num_v; ++u) {
		heap.emplace(deg[u], u);
	}

	std::vector<char> removed(Num_v, 0);

	while (!heap.empty()) {
		auto top = heap.top();
		auto d = top.first;
		auto v = top.second;
		heap.pop();
		if (removed[v]) continue;

		ordering.push_back(v);   
		core_number[v] = d;   
		removed[v] = 1;

		for (int nb : adj[v]) {
			if (!removed[nb]) {
				--deg[nb];
				heap.emplace(deg[nb], nb);
			}
		}

	}
}


int scr_upperbound_color(const std::vector<int>& nodes,
	const std::vector<int>& ordering)
{
	std::vector<char> in_nodes(Num_v, 0);
	for (int v : nodes) {
		if (v >= 0 && v < Num_v)
			in_nodes[v] = 1;
	}

	std::vector<int> color(Num_v, 0);

	for (int u : ordering) {
		if (u < 0 || u >= Num_v) continue;
		if (!in_nodes[u]) continue;  


		std::unordered_set<int> nei_colors;

		for (int v : adj[u]) {

			if (v < 0 || v >= Num_v) continue;
			if (!in_nodes[v]) continue;  
			nei_colors.insert(color[v]);
		}


		while (nei_colors.count(color[u])) {
			++color[u];
		}
	}


	int max_color = 0;
	bool has_any = false;
	for (int v : nodes) {
		if (v < 0 || v >= Num_v) continue;
		has_any = true;
		if (color[v] > max_color)
			max_color = color[v];
	}

	if (!has_any) return 0;

	return max_color;
}


// N(u)
void neighbors_of(int u, std::vector<int>& N1) {
	N1.clear();
	if (u < 0 || u >= Num_v) {
		cout << "error in neighbors_of" << endl;
		exit(-1);
		return;
	}

	for (int v : adj[u]) {
		N1.push_back(v);
	}

}


void reduction_rule_4(int k) {
	std::vector<int> ordering_rev = ordering;
	std::reverse(ordering_rev.begin(), ordering_rev.end());

	std::vector<int> Gu;
	Gu.reserve(Num_v);

	int debug_cnt = 0;

	for (int u : ordering) {
		if (Must_not_in_S[u])continue;
		if (Must_in_S[u])continue;

		Gu.clear();

		for (int v : adj[u]) {
			Gu.push_back(v);
		}

		Gu.push_back(u);

		int ub = scr_upperbound_color(Gu, ordering_rev);

		int len_Gu = (int)Gu.size();
		int len_remove = (int)remove_at_least_one[u].size();

		if (k < len_Gu - ub + len_remove) {
			Must_not_in_S[u] = 1;
		}
	}
}


//  N2(u)
void second_neighbors_of(int u, const std::vector<int>& N1, std::vector<int>& N2) {

	std::vector<char> inN1(Num_v, 0);
	for (int v : N1) {
		if (v >= 0 && v < Num_v)
			inN1[v] = 1;
		else {
			cout << "error in second_neighbors_of" << endl;
			exit(-1);
		}
	}

	std::vector<char> mark(Num_v, 0);
	N2.clear();

	for (int v : N1) {
		if (v < 0 || v >= Num_v) continue;
		for (int w : adj[v]) {
			if (w == u) continue;
			if (inN1[w]) continue;
			if (!mark[w]) {
				mark[w] = 1;
				N2.push_back(w);
			}
		}

	}
}


// 极大匹配
int greedy_maximal_matching_bipartite(const std::vector<int>& N1,
	const std::vector<int>& N2) {
	if (N1.empty() || N2.empty()) return 0;

	const int nR = (int)N2.size();

	std::vector<char> matchedR(nR, 0);

	std::vector<int> idxR(Num_v, -1);
	for (int j = 0; j < nR; ++j) {
		int v = N2[j];
		if (v >= 0 && v < Num_v)
			idxR[v] = j;
	}

	int match = 0;

	for (int v : N1) {
		if (v < 0 || v >= Num_v) continue;

		for (int w : adj[v]) {
			if (w < 0 || w >= Num_v) continue;

			int j = idxR[w];
			if (j != -1 && !matchedR[j]) {
				matchedR[j] = 1;
				++match;
				break;
			}
		}

	}

	return match;
}


void reduction_rule_5(int k)
{
	std::vector<int> N1;  // N(u)
	std::vector<int> N2;  // N2(u)

	int cnt = 0;
	for (int u : ordering) {
		if (Must_not_in_S[u])continue;
		if (Must_in_S[u])continue;

		neighbors_of(u, N1);

		second_neighbors_of(u, N1, N2);

		// |N2(u)| >= k + 1
		if ((int)N2.size() < k + 1) continue;

		int msize = greedy_maximal_matching_bipartite(N1, N2);

		if (msize > k) {
			Must_not_in_S[u] = 1;
		}
	}
}


bool has_at_least_two_clique_components(const std::vector<int>& S_nodes) {

	static std::vector<uint32_t> inS;
	static std::vector<uint32_t> visited;
	static uint32_t stamp = 1;

	if ((int)inS.size() != Num_v) {
		inS.assign(Num_v, 0);
		visited.assign(Num_v, 0);
		stamp = 1;
	}

	++stamp;
	if (stamp == 0) { 
		std::fill(inS.begin(), inS.end(), 0);
		std::fill(visited.begin(), visited.end(), 0);
		stamp = 1;
	}

	for (int v : S_nodes) {
		if (0 <= v && v < Num_v) inS[v] = stamp;
	}

	std::vector<int> st;
	std::vector<int> comp;
	st.reserve(1024);
	comp.reserve(1024);

	int clique_cnt = 0;

	for (int start : S_nodes) {
		if (start < 0 || start >= Num_v) continue;
		if (inS[start] != stamp) continue;
		if (visited[start] == stamp) continue;

		st.clear();
		comp.clear();
		st.push_back(start);
		visited[start] = stamp;

		while (!st.empty()) {
			int v = st.back();
			st.pop_back();
			comp.push_back(v);

			for (int nb : adj[v]) {
				if (inS[nb] != stamp) continue;
				if (visited[nb] == stamp) continue;
				visited[nb] = stamp;
				st.push_back(nb);
			}

		}

		if (is_clique_comp_fast(comp)) {
			++clique_cnt;
		}


		if (clique_cnt >= 2) return true;
	}

	return false;
}

void reduction_rule_6()
{
	std::vector<int> S_nodes;
	std::vector<int> N1;
	std::vector<std::vector<int>> comps;
	for (int v = 0; v < Num_v; ++v) {
		S_nodes.push_back(v);
	}

	iuc_components(S_nodes, comps);

	std::vector<int> comp_index(Num_v, -1);
	for (int ci = 0; ci < (int)comps.size(); ++ci) {
		for (int v : comps[ci]) {
			if (v >= 0 && v < Num_v)
				comp_index[v] = ci;
		}
	}

	for (int u : ordering) {
		if (Must_in_S[u])continue;
		if (Must_not_in_S[u])continue;

		int index = comp_index[u];
		if (index == -1) {
			cout << "error in index" << endl;
			exit(-1);
		}

		N1.clear();
		for (int v : comps[index]) {
			if (v == u)continue;

			N1.push_back(v);
		}

		if ((int)N1.size() < 2) continue;  

		if (has_at_least_two_clique_components(N1)) {
			Must_not_in_S[u] = 1;

		}
	}
}


void closed_neighborhood_set(int v, std::set<int>& sig) {
	sig.clear();
	sig.insert(v);
	for (int u : adj[v]) {
		sig.insert(u);
	}

}


/*
 * 关键团分组
 */
int build_critical_cliques_groups_sets(
	int*& group_of,
	std::vector<std::vector<int>>& groups,
	std::vector<int>& group_sizes
) {
	std::map<std::set<int>, std::vector<int>> buckets;
	std::set<int> sig;

	for (int v = 0; v < Num_v; ++v) {
		closed_neighborhood_set(v, sig);
		buckets[sig].push_back(v);
	}

	const int num_groups = static_cast<int>(buckets.size());

	if (group_of != nullptr) {
		delete[] group_of;
		group_of = nullptr;
	}
	group_of = new int[Num_v];
	std::fill(group_of, group_of + Num_v, -1);

	groups.clear();
	groups.reserve(num_groups);
	group_sizes.clear();
	group_sizes.reserve(num_groups);

	int gid = 0;
	for (const auto& kv : buckets) {
		const std::vector<int>& verts = kv.second;
		groups.push_back(verts);
		group_sizes.push_back(static_cast<int>(verts.size()));
		for (int v : verts) {
			group_of[v] = gid;
		}
		++gid;
	}

	return num_groups;
}


void apply_rule7_sets(
	const int* group_of,
	const std::vector<int>& group_sizes,
	int num_groups,
	int k
) {
	const int n = Num_v; 

	for (int g = 0; g < num_groups; ++g) {
		int sizeK = group_sizes[g];
		if (sizeK > k) {
			for (int v = 0; v < Num_v; ++v) {
				if (group_of[v] == g) {
					Must_in_S[v] = 1;
				}
			}
		}
	}
}


/*
 * reduction_rule_7_8_sets:
 *  - 利用闭邻域构造 critical cliques
 *  - 大关键团 → Must_in_S
 */
void reduction_rule_7_8_sets(
	int k,
	int*& group_of,
	std::vector<std::vector<int>>& groups,
	int& num_groups
) {
	std::vector<int> group_sizes;
	num_groups = build_critical_cliques_groups_sets(group_of, groups, group_sizes);

	apply_rule7_sets(group_of, group_sizes, num_groups, k);
}

