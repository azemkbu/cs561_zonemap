#include <fstream>
#include <algorithm>
#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include "args.hxx"

// Specify your path of workload file here
std::string kInputDataPath = "./workload.dat";
std::string kPointQueriesPath = "./point_queries.txt";
std::string kRangeQueriesPath = "./range_queries.txt";

// All the workload parameters
struct Parameters {
	int N; // the number of total elements to load
	uint32_t UB; //  the upper bound of the largest positive element
	int P; // the number of total point queries
	int R; // the number of total range queries
	double s; // the selectivity for each range query
	bool sort_flag; // the flag that indicates if the input workload is sorted
};

int parse_arguments(int argc, char *argv[], Parameters & params);
void generate_input_data(std::string & output_path, Parameters & params,
		std::vector<int> & input_data);
void generate_point_queries(std::string & output_path, Parameters & params,
		std::vector<int> & /*input_data*/);
void generate_range_queries(std::string & output_path, Parameters & params,
		std::vector<int> & input_data);

int main(int argc, char *argv[]) {
	Parameters params;
	if (parse_arguments(argc, argv, params)) {
		exit(1);
	}
	std::vector<int> data;
	generate_input_data(kInputDataPath, params, data);
	generate_point_queries(kPointQueriesPath, params, data);
	generate_range_queries(kRangeQueriesPath, params, data);

	return 0;
}

int parse_arguments(int argc, char *argv[], Parameters & params) {
	args::ArgumentParser parser("Zonemap_Workload_Parser.", "");
	args::Group group(parser, "Workload parameters:", args::Group::Validators::DontCare);
	args::ValueFlag<int> number_of_total_elements_cmd(group, "N",
			"The number of total elements  [def: 1000000]", {'N', "num-points"});
	args::ValueFlag<uint32_t> upper_bound_of_largest_element_cmd(group, "UB",
			"The upper bound of the largest positive element  [def: 3000000]",
			{"UB", "upper-bound-of-the-max-element"});
	args::ValueFlag<int> number_of_point_queries_cmd(group, "P",
			"The number of total point queries [def: 10000] (duplicates may exist "
			"in the input data, there might be more than one matching records)",
			{'P', "num-point-queries"});
	args::ValueFlag<int> number_of_range_queries_cmd(group, "R",
			"The number of total range queries [def: 1000]", {'R', "num-range-queries"});
	args::ValueFlag<double> selectivity_of_range_queries_cmd(group, "s",
			"The selectivity for each range queries (0 <= s <= 1), [def: 0.5]",
			{'s', "selectivity-of-range-queries"});
        args::Flag sorted_cmd(group, "is_sorted", "sort the input workload", {"sort"});

	try {
		parser.ParseCLI(argc, argv);
	}
	catch (args::Help&) {
		std::cout << parser;
		exit(0);
	}
	catch (args::ParseError& e) {
		std::cerr << e.what() << std::endl;
		std::cerr << parser;
		return 1;
	}
	catch (args::ValidationError& e) {
		std::cerr << e.what() << std::endl;
		std::cerr << parser;
		return 1;
	}

	params.N =  number_of_total_elements_cmd ? args::get(number_of_total_elements_cmd) : 1000000;
	params.P = number_of_point_queries_cmd ? args::get(number_of_point_queries_cmd) : 10000;
	params.R = number_of_range_queries_cmd ? args::get(number_of_range_queries_cmd) : 1000;
	params.UB = upper_bound_of_largest_element_cmd ?
		args::get(upper_bound_of_largest_element_cmd) : 3000000;
	params.s = selectivity_of_range_queries_cmd ?
		args::get(selectivity_of_range_queries_cmd) : 0.5;
	params.sort_flag = sorted_cmd ? args::get(sorted_cmd) : false;
	return 0;
}


void generate_input_data(std::string & output_path, Parameters & params,
		std::vector<int> & input_data) {
	input_data.clear();
	input_data.resize(params.N);
	// construct a trivial random generator engine from a time-based seed:
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine gen (seed);
	std::uniform_int_distribution<int>  dist(0, params.UB);
	std::ofstream output_file(output_path, std::ios::binary);
	if (params.sort_flag){
		for (size_t i = 0; i < params.N; i++) {
			input_data[i] = dist(gen);
		}
		sort(input_data.begin(), input_data.end());
		for (size_t i = 0; i < params.N; i++) {
        		output_file.write(reinterpret_cast<char *>(&input_data[i]), sizeof(int));
		}
	} else {
		for (size_t i = 0; i < params.N; i++) {
			input_data[i] = dist(gen);
        		output_file.write(reinterpret_cast<char *>(&input_data[i]), sizeof(int));
		}
	}
	output_file.close();
}

void generate_point_queries(std::string & output_path, Parameters & params,
		std::vector<int> & input_data) {

	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine gen (seed);
	std::uniform_int_distribution<int>  dist1(0, (size_t) (2.0*params.UB));
	std::uniform_int_distribution<int>  dist2(0, input_data.size() - 1);
	std::ofstream output_file(output_path);
	srand(time(NULL));
	for (size_t i = 0; i < params.P; i++) {
		if (rand()*1.0/RAND_MAX <= 0.2) {
			// with 0.2 probability, randomly generate point queries, may contain existing
			// or non-exising queries
			output_file << dist1(gen) << std::endl;
		} else {
			// generate existing queries
			output_file << input_data[dist2(gen)] << std::endl;
		}
	}
	// The above process may produce duplicate point queries. And the number of exsiting quries
	// has 0.8*P as the lower bound.
	output_file.close();

}
void generate_range_queries(std::string & output_path, Parameters & params,
							std::vector<int> & input_data) {
	std::ofstream output_file(output_path);
	if (!output_file.is_open()) return;

	const size_t N = input_data.size();
	if (N == 0 || params.R <= 0) return;

	size_t k = static_cast<size_t>(params.s * static_cast<double>(N));

	if (params.s > 0.0 && k == 0) k = 1;

	if (k > N) k = N;

	std::vector<int> sorted_data = input_data;
	std::sort(sorted_data.begin(), sorted_data.end());

	std::mt19937 gen(std::random_device{}());

	if (k == N) {
		for (int i = 0; i < params.R; i++) {
			output_file << sorted_data.front() << " " << sorted_data.back() << "\n";
		}
		return;
	}

	std::uniform_int_distribution<size_t> dist_start(0, N - k);

	for (int i = 0; i < params.R; i++) {
		size_t start = dist_start(gen);
		int low  = sorted_data[start];
		int high = sorted_data[start + k - 1];
		output_file << low << " " << high << "\n";
	}
	output_file.close();
}



