#include "Utility.h"
#include "Graph.h"

void print_usage() {
	printf("Usage: RLS graph-file [solution-file] [time-seconds] [seed]\n");
}

int main(int argc, char *argv[]) {
	if(argc < 2) {
		print_usage();
		return 0;
	}


#ifdef _LINUX_
	struct timeval start, end1, end;
	gettimeofday(&start, NULL);
#endif
	
	Graph *graph = new Graph(argv[1]);
	int time_seconds = argc >= 4 ? std::max(1, atoi(argv[3])) : 30;
	unsigned seed = argc >= 5 ? static_cast<unsigned>(strtoul(argv[4], nullptr, 10)) : 0U;
	graph->set_search_options(time_seconds, 1, seed);
	graph->read_graph_GIS();
	// graph->read_graph_GIS2();
#ifdef _LINUX_
	gettimeofday(&end1, NULL);

	long long mtime1, seconds1, useconds1;
	seconds1 = end1.tv_sec - start.tv_sec;
	useconds1 = end1.tv_usec - start.tv_usec;
	mtime1 = seconds1*1000000 + useconds1;
#endif

	graph->GIS();
	if (argc >= 3) graph->write_solution(argv[2]);
	printf("RLS_BEST_WEIGHT: %d\n", graph->solution_value);
#ifdef _LINUX_
	gettimeofday(&end, NULL);

	long long mtime, seconds, useconds;
	seconds = end.tv_sec - start.tv_sec;
	useconds = end.tv_usec - start.tv_usec;
	mtime = seconds*1000000 + useconds;

	//printf("Total time excluding IO is: %lld\n", mtime-mtime1);
#endif

	return 0;
}
