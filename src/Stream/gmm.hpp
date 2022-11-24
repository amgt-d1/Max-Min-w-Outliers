#pragma once
#include "../utils/data.hpp"
#include <functional>
#include <algorithm>


// algorithm
class gmm
{
    // dataset in instance
    std::vector<point> pset;

    // seed
    unsigned int seed = 0;

    // solution set
    std::vector<unsigned int> solution;

    // vector for dist-min
    std::vector<float> dist_min_array;

    // running time
    double running_time = 0;

    // radius
    float radius = 0;

    // #outliers in centers
    double outlier_num = 0;

    
    // distance computation
    float compute_distance(const point *l, const point *r)
    {
        float distance = 0;
        for (unsigned int i = 0; i < dimensionality; ++i) distance += (l->pt[i] - r->pt[i]) * (l->pt[i] - r->pt[i]);
        return sqrt(distance);
    }

public:
    // constructor
    gmm() {}

    gmm(unsigned int s)
    {
        // init
        seed = s;
        pset = point_set;
        dist_min_array.resize(pset.size());

        const unsigned int size = pset.size();
        for (unsigned int i = 0; i < size; ++i) dist_min_array[i] = FLT_MAX;
    }

    // destructor
    ~gmm()
    {
        pset.shrink_to_fit();
        solution.shrink_to_fit();
        dist_min_array.shrink_to_fit();
    }

    // Max-Min Diversification
    void diversification()
    {
        start = std::chrono::system_clock::now();

        // get size
        const unsigned int size = pset.size();

        // random generator
        std::mt19937 mt(seed);
        std::uniform_int_distribution<> rnd_int(0, size - 1);

        /******************/
        /*** 1st sample ***/
        /******************/
        unsigned int idx = rnd_int(mt);
        solution.push_back(idx);

        /**********************/
        /*** i-th iteration ***/
        /**********************/
        for (unsigned int i = 1; i < k + z; ++i)
        {
            std::pair<unsigned int, float> candidate = {0,0};

            // get last center
            point* p = &pset[idx];

            // update (1) dist-min to the intermediate result & (2) candidate
            for (unsigned int j = 0; j < size; ++j)
            {
                // (1) dist-min update
                const float distance = compute_distance(p, &pset[j]);
                if (dist_min_array[j] > distance) dist_min_array[j] = distance;

                // (2) candidate update
                if (dist_min_array[j] > candidate.second) candidate = {j, dist_min_array[j]};
            }

            // result update
            solution.push_back(candidate.first);
            radius = candidate.second;
            idx = candidate.first;
        }

        end = std::chrono::system_clock::now();
        running_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        running_time /= 1000;
    }

    float get_radius() { return radius; }
};
