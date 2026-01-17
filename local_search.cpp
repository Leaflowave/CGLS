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


// ------------------------------------------------------------------
//  iuc_components
//  Compute connected components of the induced subgraph G[S_nodes].
// ------------------------------------------------------------------
#include <vector>
#include <cstdint>
#include <algorithm>

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

			for (int ei = pstart[v]; ei < pstart[v + 1]; ++ei) {
				int u = edges[ei];
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

	//int comp_cnt = 0;
	////Validate initial S.,判断是否是iuc
	//if (!validate_iuc(S, comp_cnt)) {
	//	std::cout << "Input S is not an IUC: " << std::endl;
	//	std::exit(-1);
	//}

	// 计算当前 G[S] 的连通分量，并建立顶点→分量索引
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

	// 不断向 S 加点，直到极大
	std::vector<int> comp_size(comps.size(), 0);
	for (int ci = 0; ci < (int)comps.size(); ++ci) comp_size[ci] = (int)comps[ci].size();

	std::vector<int> cover_cnt(Num_v, 0);     
	std::vector<int> touch_comp(Num_v, -1);   
	std::vector<int> touch_cnt(Num_v, 0);    
	std::vector<uint8_t> touch_multi(Num_v, 0);

	std::vector<int> uncovered;             
	std::vector<int> pos_uncovered(Num_v, -1);

	std::vector<int> joinable;                
	std::vector<int> active;          

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
		if (v < 0 || v >= Num_v) {
			cout << "error in is_joinable" << endl;
			return false;
		}
		if (Must_not_in_S[v]) return false;
		if (inS[v]) return false;
		if (touch_multi[v]) return false;
		int cid = touch_comp[v];
		if (cid < 0 || cid >= (int)comp_size.size()) return false;
		return touch_cnt[v] == comp_size[cid];
		};

	auto push_joinable_if = [&](int v) {
		if (is_joinable(v)) joinable.push_back(v);
		};


	for (int s : S) {
		int cid = comp_index[s];
		for (int ei = pstart[s]; ei < pstart[s + 1]; ++ei) {
			int x = edges[ei];
			if (x < 0 || x >= Num_v) {
				cout << "error in comp_index" << endl;
				exit(-1);
				continue;
			}
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
	for (int v = 0; v < Num_v; ++v) {
		if (Must_not_in_S[v]) continue;
		if (inS[v]) continue;
		if (cover_cnt[v] == 0) add_to_uncovered(v);
	}


	joinable.reserve(100000);
	for (int v = 0; v < Num_v; ++v) {
		if (cover_cnt[v] == 0) continue;
		push_joinable_if(v);
	}

	// --- 3) 增量加入点：更新邻居
	auto add_vertex_to_S = [&](int v, int cid) {

		if (inS[v]) {
			cout << "error in add" << endl;
			exit(-1);
		}

		inS[v] = 1;
		S.push_back(v);

		remove_from_uncovered(v);


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

		for (int ei = pstart[v]; ei < pstart[v + 1]; ++ei) {
			int x = edges[ei];
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

			push_joinable_if(x);
		}
		};

	while (true) {
		bool progressed = false;
		while (!joinable.empty()) {
			int v = joinable.back();
			joinable.pop_back();

			if (!is_joinable(v)) continue;       
			int cid = touch_comp[v];              
			add_vertex_to_S(v, cid);
			progressed = true;
			break; 
		}
		if (progressed) continue;

		if (!uncovered.empty()) {
			int idx = std::rand() % uncovered.size();
			int v = uncovered[idx];

			if (Must_not_in_S[v] || inS[v] || cover_cnt[v] != 0)
			//if (inS[v] || cover_cnt[v] != 0) 
			{
				remove_from_uncovered(v);
				continue;
			}
			add_vertex_to_S(v, -1);
			continue;
		}

		break;
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
	for (int ei = pstart[v]; ei < pstart[v + 1]; ++ei) {
		nei.push_back(edges[ei]);
	}
}

// count |S ∩ N(v)|
static int count_neighbors_in_set(int v, const unordered_set<int>& S) {
	int cnt = 0;
	for (int ei = pstart[v]; ei < pstart[v + 1]; ++ei) {
		if (S.count(edges[ei])) ++cnt;
	}
	return cnt;
}

static std::unordered_set<int> get_nei(int v) {
	std::unordered_set<int> nei;
	for (int i = pstart[v]; i < pstart[v + 1]; ++i) {
		nei.insert(edges[i]);
	}
	return nei;
}

// |cc ∩ N(v)|
static int count_neighbors_in_comp(int v, const unordered_set<int>& cc) {
	int cnt = 0;
	for (int ei = pstart[v]; ei < pstart[v + 1]; ++ei) {
		if (cc.count(edges[ei])) ++cnt;
	}
	return cnt;
}


static inline std::unordered_set<int> neighbors_of(int v) {
	std::unordered_set<int> nei;
	for (int i = pstart[v]; i < pstart[v + 1]; ++i) {
		nei.insert(edges[i]);
	}
	return nei;
}


struct VertexInfo {
	int nbr_in_S = 0;                         // |N(i) ∩ S|
	std::unordered_map<int, int> nbr_in_C;    // cid -> |N(i) ∩ C|
	int conflict = -1;                         // 当前冲突数 conflict(i)
	int min_conflict_cluster = -1;            // 当前最小冲突的cluster id
	int judge_confict = 0;
};

std::vector<VertexInfo> vertex_info; // size = Num_v
std::vector<std::unordered_set<int>> clusters;  // 每个 cluster C


static void rebuild_from_S_conflict(const std::unordered_set<int>& S)
{
	// 1️⃣ 初始化 / 重置
	if ((int)vertex_info.size() != Num_v) {
		cout << "error in vertex_info" << endl;
		exit(-1);
	}


	clusters.clear();
	clusters.reserve(S.size());

	// 2️⃣ 计算 G[S] 的连通分量（每个分量即一个 clique）
	std::vector<int> S_nodes(S.begin(), S.end());
	std::vector<std::vector<int>> comps;
	iuc_components(S_nodes, comps);  // 复用原有的连通分量函数

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

		for (int ei = pstart[u]; ei < pstart[u + 1]; ++ei) {
			int v = edges[ei]; 

			vertex_info[v].nbr_in_S++;

			if (cid != -1) {
				vertex_info[v].nbr_in_C[cid]++;
			}
		}
	}

	// 计算 conflict 与 min_conflict_cluster
	for (int v = 0; v < Num_v; ++v) {
		vertex_info[v].judge_confict = 0;
		if (S.count(v)) continue;  

		bool flag = false;//判断是否是PA / IS
		bool st = false;//判断是否是OM / OA
		int nbrS = vertex_info[v].nbr_in_S;
		int best_conflict = nbrS;
		int best_cid = -1;
		int judge_conf = 0;

		// 遍历所有簇，寻找最小冲突
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


	for (int ei = pstart[v]; ei < pstart[v + 1]; ++ei) {
		int u = edges[ei];
		//if (S.count(u)) continue;

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

	for (int ei = pstart[v]; ei < pstart[v + 1]; ++ei) {
		int u = edges[ei];
		//if (S.count(u)) continue;
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

// 只检查 S_nodes 是否是 IUC，返回 true/false
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



bool check_IUC_validity(const std::vector<std::unordered_set<int>>& clusters)
{
	for (int cid = 0; cid < (int)clusters.size(); ++cid) {
		const auto& C = clusters[cid];
		for (int u : C) {
			for (int v : C) {
				if (u >= Num_v || v >= Num_v) {
					std::cerr << "[IUC-Check] Invalid vertex index: "
						<< u << " or " << v
						<< " in cluster " << cid << std::endl;
					return false;
				}
				if (u == v) continue;
				if (!hasEdge(u, v)) {
					std::cerr << "[IUC-Check] ❌ Intra-clique violation: vertices "
						<< u << " and " << v
						<< " are not connected inside cluster " << cid << std::endl;
					return false;
				}
			}
		}
	}

	// ===============================
	// 2️⃣ 检查不同 cluster 是否独立（inter-clique independence）
	// ===============================
	for (int i = 0; i < (int)clusters.size(); ++i) {
		for (int j = i + 1; j < (int)clusters.size(); ++j) {
			for (int u : clusters[i]) {
				for (int v : clusters[j]) {
					if (hasEdge(u, v)) {
						std::cerr << "[IUC-Check] ❌ Inter-clique violation: "
							<< "edge (" << u << ", " << v
							<< ") exists between cluster " << i
							<< " and cluster " << j << std::endl;
						return false;
					}
				}
			}
		}
	}

	// ===============================
	// 3️⃣ 检查是否有重复顶点出现在多个 cluster 中
	// ===============================
	std::vector<int> appear(Num_v, 0);
	for (int cid = 0; cid < (int)clusters.size(); ++cid) {
		for (int v : clusters[cid]) {
			if (v < 0 || v >= Num_v) {
				std::cerr << "[IUC-Check] ❌ Invalid vertex id " << v
					<< " in cluster " << cid << std::endl;
				return false;
			}
			appear[v]++;
			if (appear[v] > 1) {
				std::cerr << "[IUC-Check] ❌ Vertex " << v
					<< " appears in multiple clusters!" << std::endl;
				return false;
			}
		}
	}

	// ===============================
	// 4️⃣ 若全部通过，返回 true
	// ===============================
	/*std::cout << "[IUC-Check]  Current solution is a valid IUC ("
		<< clusters.size() << " clusters)" << std::endl;*/
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
	//if (Num_v > 500) {
	//	max_depth = Num_v / 2;
	//}


	// 4️⃣ 主循环
	while (depth < max_depth)
	{
		if ((double)(clock() - Start_time) / CLOCKS_PER_SEC >= Time_limit) {
			if ((int)S.size() > best_size) {
				Run_time = (double)(clock() - Start_time) / CLOCKS_PER_SEC;
				best_size = (int)S.size();
				best_S.assign(S.begin(), S.end());
			}
			break;
		}
		++depth;
		bool improved = false;

		// 所有 tabu 减 1
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

			//int vin;
			//if (!cand_IS.empty()) {
			//	//cout << "OM" << endl;
			//	vin = weighted_choice_exp(cand_IS, freq);
			//}
			//else if (!cand_PA.empty()) {
			//	vin = weighted_choice_exp(cand_PA, freq);
			//}
			int vin = weighted_choice_exp(cand_add, freq);
			//int vin = cand_add[std::rand() % cand_add.size()];
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
				//int vin = weighted_choice_exp(cand_swap, freq);

				if (!cand_OM.empty())
					//if(vertex_info[vin].judge_confict == 2)
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
					/*if (in_cc == csize - 1) {*/
					for (int u : C) {
						//if (Must_not_in_S[u])continue;
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

					// ② OA：vin 与 C 全连，但与 C 外某点冲突
					//if (in_cc == csize && vertex_info[vin].nbr_in_S == csize + 1) {
					for (int ei = pstart[vin]; ei < pstart[vin + 1]; ++ei) {
						int u = edges[ei];
						//if (Must_not_in_S[u])continue;
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
				//int vin = weighted_choice_exp(cand_swap, freq);
				//int vin = cand_swap[std::rand() % cand_swap.size()];

			}
		}
		// -----------------------------
		// Step 3️⃣ : 更新最优解
		// -----------------------------
		if ((int)S.size() > cur_best) {
			cur_best = (int)S.size();
			best_S_vec.assign(S.begin(), S.end());
			depth = 0;
		}

		// -----------------------------
		// Step 4️⃣ : 无改进 → 扰动重启
		// -----------------------------
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

	//std::cout << "size of random maximal IUC: " << S.size() << " " << (double)(clock() - Start_time) / CLOCKS_PER_SEC << std::endl;


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
			vector<double> w(Num_v);
			vector<int> empty_init;

			for (int v = 0; v < Num_v; ++v) {
				w[v] = (double)freq[v];
				if (Must_in_S[v]) {
					empty_init.push_back(v);
				}
			}

			init_sol = random_maximal_iuc(empty_init, &w);
			//cout << "init_sol.size()" << init_sol.size() << endl;
			
			if (K_opt == (int)init_sol.size() || Run_time >= Time_limit) {
				return;
			}			

			nonImprove = 1;
		}

		vector<int> copy_init_sol = init_sol;
		nonImprove += 1;

		// 如果 freq 总和太大，就把它们减半
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

		// 更新 freq
		unordered_set<int> res_set;
		res_set.reserve(res.size() * 2);
		for (int v : res) res_set.insert(v);
		for (int v : copy_init_sol) {
			if (!res_set.count(v)) {
				// 如果某个顶点在初始解里，但最终解里没出现
				if (v >= 0 && v < Num_v) {
					freq[v] += 1;
				}
				else {
					cout << "erroe in update freq" << endl;
					exit(-1);
				}
			}
		}

		// test
	/*	if (cur_best >= best) {
			
		}*/

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
	if (pstart[u + 1] - pstart[u] > pstart[v + 1] - pstart[v]) {
		int t = u;
		u = v;
		v = t;
	}

	for (int i = pstart[u]; i < pstart[u + 1]; ++i) {
		if (edges[i] == v) return true;
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
		for (int ei = pstart[v]; ei < pstart[v + 1]; ++ei) {
			int w = edges[ei];
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
		//if (Must_not_in_S[start]) continue;
		if (visited[start] == vstamp) continue;

		st.clear();
		comp.clear();
		st.push_back(start);
		visited[start] = vstamp;

		while (!st.empty()) {
			int v = st.back();
			st.pop_back();
			comp.push_back(v);

			for (int ei = pstart[v]; ei < pstart[v + 1]; ++ei) {
				int u = edges[ei];
				//if (Must_not_in_S[u]) continue;
				if (visited[u] == vstamp) continue;
				visited[u] = vstamp;
				st.push_back(u);
			}
		}

		// Reduction Rule 1: 如果该分量是 clique，则全部 Must_in
		if (is_clique_comp_fast(comp)) {
			for (int v : comp) Must_in_S[v] = 1;
		}
	}
}


// 归约规则 2：更新 Must_in_S 集合
void reduction_rule_2() {

	// (A) deg(v)=2 且存在 deg(x)=1 邻居: IN {v,x}, OUT {y}
	for (int v = 0; v < Num_v; ++v) {
		if (Must_not_in_S[v]) continue;

		int dv = pstart[v + 1] - pstart[v];
		if (dv != 2) continue;

		int a = edges[pstart[v]];
		int b = edges[pstart[v] + 1];

		int x = -1, y = -1;

		int da = pstart[a + 1] - pstart[a];
		int db = pstart[b + 1] - pstart[b];

		if (!Must_not_in_S[a] && da == 1) { x = a; y = b; }
		else if (!Must_not_in_S[b] && db == 1) { x = b; y = a; }
		else continue;

		if (Must_not_in_S[x]) continue;
		if (Must_in_S[y]) continue;     // 不能把已经 IN 的点 OUT

		Must_in_S[v] = 1;
		Must_in_S[x] = 1;
		Must_not_in_S[y] = 1;
	}

	// (B) deg(u)=1: IN {u}, OUT {neighbor}
	for (int u = 0; u < Num_v; ++u) {
		if (Must_not_in_S[u]) continue;

		int du = pstart[u + 1] - pstart[u];
		if (du != 1) continue;

		int v = edges[pstart[u]]; // 唯一邻居

		if (Must_in_S[v]) continue;  // v 已经 IN，不能 OUT v

		int dv = pstart[v + 1] - pstart[v];

		if (dv == 1) {
			Must_in_S[u] = 1;
			continue;
		}

		Must_in_S[u] = 1;
		Must_not_in_S[v] = 1;
	}
}


// k + 1个共同邻居
void reduction_rule_3(int k)
{
	forbidden_diff.clear();
	remove_at_least_one.clear();

	// 标记数组：mark[w] == 1 表示当前 u 的邻居里有 w
	std::vector<char> mark(Num_v, 0);

	int test_cnt = 0;
	// 遍历所有节点对 (u, v)
	for (int u = 0; u < Num_v; ++u) {
		if (Degree[u] <= k) continue;

		for (int ei = pstart[u]; ei < pstart[u + 1]; ++ei) {
			int w = edges[ei];
			mark[w] = 1;
		}

		for (int v = u + 1; v < Num_v; ++v) {
			if (Degree[v] <= k) continue;

			int common = 0;

			for (int ei = pstart[v]; ei < pstart[v + 1]; ++ei) {
				int w = edges[ei];
				if (mark[w]) {
					++common;
					if (common >= k + 1) break;  
				}
			}

			// 3) 如果交集大小 >= k + 1，则满足条件
			if (common >= k + 1) {
				forbidden_diff.insert({ u, v });  // 将 (u, v) 加入 forbidden_diff

				// 如果 u 和 v 之间没有边，则进行处理
				if (!hasEdge(u, v)) {
					remove_at_least_one[u].insert(v);
					remove_at_least_one[v].insert(u);
					++test_cnt;
				}
			}
		}

		// 4) 清除 u 的邻居标记，为下一个 u 做准备
		for (int ei = pstart[u]; ei < pstart[u + 1]; ++ei) {
			int w = edges[ei];
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
		deg[u] = pstart[u + 1] - pstart[u];  // 邻接表中边数量
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

		ordering.push_back(v);   // 加到退化序
		core_number[v] = d;      // 记录核心数
		removed[v] = 1;

		for (int ei = pstart[v]; ei < pstart[v + 1]; ++ei) {
			int nb = edges[ei];
			if (!removed[nb]) {
				--deg[nb];
				heap.emplace(deg[nb], nb);
			}
		}
	}
}


// ===== 贪心染色上界（返回最大颜色编号）=====
int scr_upperbound_color(const std::vector<int>& nodes,
	const std::vector<int>& ordering)
{
	// 1️⃣ 标记子图中的点：in_nodes[u] == 1 表示 u ∈ nodes
	std::vector<char> in_nodes(Num_v, 0);
	for (int v : nodes) {
		if (v >= 0 && v < Num_v)
			in_nodes[v] = 1;
	}

	// 2️⃣ 初始化颜色：对所有顶点先置 0
	std::vector<int> color(Num_v, 0);

	// 3️⃣ 按给定顺序做贪心染色
	for (int u : ordering) {
		if (u < 0 || u >= Num_v) continue;
		if (!in_nodes[u]) continue;  // if u not in g: continue

		// 收集 u 在子图中的邻居颜色
		std::unordered_set<int> nei_colors;
		for (int ei = pstart[u]; ei < pstart[u + 1]; ++ei) {
			int v = edges[ei];
			if (v < 0 || v >= Num_v) continue;
			if (!in_nodes[v]) continue;   // 只看 nodes 内的邻居
			nei_colors.insert(color[v]);
		}

		// while color[u] in nei_colors: color[u] += 1
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

	// 若子图为空，返回 0
	if (!has_any) return 0;

	return max_color;
}


// 取顶点 u 的一阶邻居 N(u)
void neighbors_of(int u, std::vector<int>& N1) {
	N1.clear();
	if (u < 0 || u >= Num_v) {
		cout << "error in neighbors_of" << endl;
		exit(-1);
		return;
	}
	for (int ei = pstart[u]; ei < pstart[u + 1]; ++ei) {
		int v = edges[ei];
		N1.push_back(v);
	}
}


// reduction_rule_4：基于贪心染色上界的删点规则,如果选择u，则会删除多少个点
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

		for (int ei = pstart[u]; ei < pstart[u + 1]; ++ei) {
			int v = edges[ei];
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
		for (int ei = pstart[v]; ei < pstart[v + 1]; ++ei) {
			int w = edges[ei];
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

		for (int ei = pstart[v]; ei < pstart[v + 1]; ++ei) {
			int w = edges[ei];
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


// 在诱导子图 G[S_nodes] 上：统计 clique 连通分量个数是否 >= 2
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

			for (int ei = pstart[v]; ei < pstart[v + 1]; ++ei) {
				int nb = edges[ei];
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
			//if (Must_not_in_S[v])continue;
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
	for (int ei = pstart[v]; ei < pstart[v + 1]; ++ei) {
		int u = edges[ei];
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

	// Rule 7：把足够大的关键团加入 Must_in_S
	apply_rule7_sets(group_of, group_sizes, num_groups, k);
}

