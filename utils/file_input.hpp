#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>
#include <unordered_map>
#include <chrono>


// stats
double time_avg = 0;
double radius_avg = 0;
double radius_sqr = 0;
double outlier_num_avg = 0;
std::chrono::system_clock::time_point start, end;


// dataset identifier
unsigned int dataset_id = 0;

// data dimensionality
unsigned int dimensionality = 2;

// summary size
unsigned int k = 2;

// outlier number
float z = 0;

// cardinality for synthetic data
unsigned int cardinality = 0;

// run number
const unsigned int run_num = 20;

// get current time
void get_current_time()
{
	time_t t = time(NULL);
	printf(" %s\n\n", ctime(&t));
}

// parameter input
void input_parameter()
{
	std::ifstream ifs_dataset_id("parameter/dataset_id.txt");
	std::ifstream ifs_k("parameter/k.txt");
	std::ifstream ifs_z("parameter/z.txt");

	if (ifs_dataset_id.fail())
	{
		std::cout << " dataset_id.txt does not exist." << std::endl;
		std::exit(0);
	}
	if (ifs_k.fail())
	{
		std::cout << " k.txt does not exist." << std::endl;
		std::exit(0);
	}
	if (ifs_z.fail())
	{
		std::cout << " z.txt does not exist." << std::endl;
		std::exit(0);
	}

	while (!ifs_dataset_id.eof()) { ifs_dataset_id >> dataset_id; }
	while (!ifs_k.eof()) { ifs_k >> k; }
	while (!ifs_z.eof()) { ifs_z >> z; }

	// set dimensionality
	if (dataset_id == 0) dimensionality = 2;
	if (dataset_id == 1) dimensionality = 7;
	if (dataset_id == 2) dimensionality = 10;
	if (dataset_id == 3) dimensionality = 16;
	if (dataset_id == 4) dimensionality = 115;
}