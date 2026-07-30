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
#include <string>

using namespace std;

#define MAXCAN 1024
#define MAXNUM 99999999
#define PRECISION 1.0e-6
#define TRUE 1
#define FALSE 0
#define TRISTATE -1

extern char *Instance_name;		//instance name
extern vector<vector<int>> adj;   // �ڽӱ�

extern  vector<int> Degree; //degree of each vertex
extern int Num_v;				//number of vertices
extern long long Num_e;               //number of edges
extern double Density;          //density of graph
extern int K_opt;               //optimal size of IUC (available for some instances)

extern int Compo_cnt;  // ��ͨ��������

extern std::vector<int> Must_in_S;      // �Ƿ����ѡ
extern std::vector<int> Must_not_in_S;  // �Ƿ��ֹѡ

extern set<pair<int, int>> forbidden_diff;  // �洢�������Ľڵ��
extern unordered_map<int, unordered_set<int>> remove_at_least_one;  // �洢ÿ���ڵ���Ҫɾ�����ھ�

extern int* degree;
extern vector<int> ordering;
extern unordered_map<int, int> core_number;  // �洢ÿ���ڵ�ĺ�����


extern double Time_limit, Start_time, Run_time;

extern int best_size;
extern std::vector<int> best_S;

extern int* group_of;
extern std::vector<std::vector<int>> groups;
extern int num_groups;


struct CGColumn {
    std::vector<int> vertices;   // sorted vertex ids of a clique
    double score = 0.0;          // heuristic score used by column-level search
};

extern std::vector<CGColumn> columns;

extern int sum_it;
extern int Column_MWIS_RLS_Runs;
extern int Column_MWIS_RLS_Used;
extern int Column_MWIS_RLS_Improved;
extern int Column_MWIS_RLS_Selected_Columns;
extern int Column_MWIS_RLS_Weight;
extern int Column_MWIS_RLS_Input_Columns;
extern int Column_MWIS_RLS_Exit_Code;
extern int Column_MWIS_RLS_Timed_Out;
extern int Column_MWIS_RLS_Input_Max_Weight;
extern int Column_MWIS_RLS_Incumbent_Components;
extern int Column_MWIS_RLS_Incumbent_Columns;
extern int Column_MWIS_RLS_Incumbent_Weight;
extern int Column_MWIS_RLS_Incumbent_Superset_Components;
extern int Column_MWIS_RLS_Incumbent_Superset_Columns;
extern int Column_MWIS_RLS_Incumbent_Superset_Vertices;
extern int Column_MWIS_RLS_Solver_Selected_Columns;
extern int Column_MWIS_RLS_Incumbent_Candidate_Columns;
extern int Column_MWIS_RLS_Incumbent_Candidate_Used;
extern int Column_MWIS_RLS_Repaired_Size;
extern double Column_MWIS_RLS_Time;
extern double Column_MWIS_Build_Time;
extern long long Column_MWIS_Conflict_Edges;

#endif
