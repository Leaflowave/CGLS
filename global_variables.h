#ifndef _GLOBAL_VARIABLES_H
#define _GLOBAL_VARIABLES_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string.h>
#include <math.h>
#include <time.h>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

#define MAXCAN 1024
#define MAXNUM 99999999
#define PRECISION 1.0e-6
#define TRUE 1
#define FALSE 0
#define TRISTATE -1

extern char *Instance_name;		//instance name
extern vector<vector<int>> adj;   // 邻接表

extern  vector<int> Degree; //degree of each vertex
extern int Num_v;				//number of vertices
extern int Num_e;               //number of edges
extern double Density;          //density of graph
extern int K_opt;               //optimal size of IUC (available for some instances)

extern int Compo_cnt;  // 连通分量数量

extern std::vector<int> Must_in_S;      // 是否必须选
extern std::vector<int> Must_not_in_S;  // 是否禁止选

extern set<pair<int, int>> forbidden_diff;  // 存储不允许的节点对
extern unordered_map<int, unordered_set<int>> remove_at_least_one;  // 存储每个节点需要删除的邻居

extern int* degree;
extern vector<int> ordering;
extern unordered_map<int, int> core_number;  // 存储每个节点的核心数


extern double Time_limit, Start_time, Run_time;

extern int best_size;
extern std::vector<int> best_S;

extern int* group_of;
extern std::vector<std::vector<int>> groups;
extern int num_groups;


#endif
