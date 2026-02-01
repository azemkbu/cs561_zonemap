all: main workloadgenerator

main: main.cpp zonemaps.cpp
	g++ -g -std=c++11 -o $@ $^

workloadgenerator: workload_generator.cpp
	g++ -g -std=c++11 -o $@ $^

clear:
	rm -f main workloadgenerator workload.dat point_queries.txt range_queries.txt

# ---------------- Workloads ----------------
# W1: 5M sorted inserts, 10K point queries
w1: all
	./workloadgenerator -N 5000000 -P 10000 -R 0 --sort
	./main

# W2: 5M sorted inserts, 1K range queries, selectivity 0.001
w2: all
	./workloadgenerator -N 5000000 -P 0 -R 1000 -s 0.001 --sort
	./main

# W3: 5M sorted inserts, 1K range queries, selectivity 0.1
w3: all
	./workloadgenerator -N 5000000 -P 0 -R 1000 -s 0.1 --sort
	./main

# W4: W1 with unsorted data
w4: all
	./workloadgenerator -N 5000000 -P 10000 -R 0
	./main

# W5: W2 with unsorted data
w5: all
	./workloadgenerator -N 5000000 -P 0 -R 1000 -s 0.001
	./main

# W6: W3 with unsorted data
w6: all
	./workloadgenerator -N 5000000 -P 0 -R 1000 -s 0.1
	./main

test_all: w1 w2 w3 w4 w5 w6
