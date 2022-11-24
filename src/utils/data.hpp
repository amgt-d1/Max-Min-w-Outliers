#pragma once
#include "file_input.hpp"
#include <cmath>
#include <cfloat>
#include <random>
#include <string>


// definition of a point
class point
{

public:

    // identifier
    unsigned int identifier = 0;

    // vector
    std::vector<float> pt;

    // flag (inlier or outlier)
    bool in = 1;    // 1: inlier

    // constructor
    point() {}

    point(const unsigned int id, std::vector<float> &p)
    {
        identifier = id;
        pt = p;
    }
};

// dataset
std::vector<point> point_set, inlier_set;

// input dataset
void input_dataset()
{
    // id variable
	unsigned int id = 0;

	// point coordinates variable
	point p;
    p.pt.resize(dimensionality);

    // min & max
    std::vector<float> min(dimensionality), max(dimensionality);
    for (unsigned int i = 0; i < dimensionality; ++i)
    {
        min[i] = FLT_MAX;
        max[i] = FLT_MIN;
    }

    // dataset input
	std::string f_name = "../_dataset/";
    if (dataset_id == 1) f_name += "household.csv";
    if (dataset_id == 2) f_name += "covertype.csv";
    if (dataset_id == 3) f_name += "kdd.csv";
    if (dataset_id == 4) f_name += "mirai.csv";

    // file input
	std::ifstream ifs_file(f_name);
    std::string full_data;

	// error check
	if (ifs_file.fail())
    {
		std::cout << " data file does not exist." << std::endl;
		std::exit(0);
	}

    // read data
	while (std::getline(ifs_file, full_data))
    {
		std::string meta_info;
		std::istringstream stream(full_data);
		std::string type = "";

		for (unsigned int i = 0; i < dimensionality; ++i)
        {
			std::getline(stream, meta_info, ',');
			std::istringstream stream_(meta_info);

            if (i < dimensionality)
            {
                const float val = (float)std::stold(meta_info);
                p.pt[i] = val;

                if (val < min[i]) min[i] = val;
                if (val > max[i]) max[i] = val;
            }
		}

		// update id
		p.identifier = id;

		// insert into dataset
		point_set.push_back(p);

		// increment identifier
		++id;
	}

    // shift max
    for (unsigned int i = 0; i < dimensionality; ++i) max[i] -= min[i];

    // get cardinality
    const unsigned int size = (unsigned int)point_set.size();

    // normalization
    const float max_coordinate = 100;
    for (unsigned int i = 0; i < size; ++i)
    {
        for (unsigned int j = 0; j < dimensionality; ++j)
        {
            point_set[i].pt[j] -= min[j];
            point_set[i].pt[j] /= max[j];
            point_set[i].pt[j] *= max_coordinate;
        }
    }

    // copy
    inlier_set = point_set;

    // random generator
    std::mt19937 mt(0);
	std::uniform_real_distribution<> rnd(-2 * max_coordinate, 3 * max_coordinate);
    std::uniform_int_distribution<> rnd_dim(0, dimensionality - 1);
    std::uniform_int_distribution<> rnd_add(2 * max_coordinate, 10 * max_coordinate);

    // outlier injection
    p.in = 0;
    for (unsigned i = 0; i < z; ++i)
    {
        while (1)
        {
            for (unsigned int j = 0; j < dimensionality; ++j) p.pt[j] = rnd(mt);

            unsigned int dim = rnd_dim(mt);
            if (p.pt[dim] > 0)
            {
                p.pt[dim] += rnd_add(mt);
            }
            else
            {
                p.pt[dim] -= rnd_add(mt);
            }
            break;
        }

        point_set.push_back(p);
    }

    // get cardinality
    cardinality = point_set.size() - z;

    std::cout << " ------------------\n";
	std::cout << " data id: " << dataset_id << "\n";
	std::cout << " dimensionality: " << dimensionality << "\n";
	std::cout << " cardinality: " << cardinality << "\n";
	std::cout << " k: " << k << "\n";
	std::cout << " z: " << z << "\n";
	std::cout << " ------------------\n\n";
}
