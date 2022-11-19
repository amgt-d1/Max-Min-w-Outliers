#include "../utils/data.hpp"
#include <functional>
#include <algorithm>


double run_avg_time_collect = 0;
double run_avg_time_remove = 0;
double run_avg_time_gmm = 0;


// algorithm
class gmm_z {

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
  
    // distance computation
    float compute_distance(const point *l, const point *r) {

        float distance = 0;
        for (unsigned int i = 0; i < dimensionality; ++i) distance += (l->pt[i] - r->pt[i]) * (l->pt[i] - r->pt[i]);
        return sqrt(distance);
    }

public:

    // constructor
    gmm_z() {}

    gmm_z(unsigned int s)
    {
        // init
        seed = s;
        pset = point_set;
        dist_min_array.resize(pset.size());

        const unsigned int size = pset.size();
        for (unsigned int i = 0; i < size; ++i) dist_min_array[i] = FLT_MAX;
    }

    // destructor
    ~gmm_z()
    {
        pset.shrink_to_fit();
        solution.shrink_to_fit();
        dist_min_array.shrink_to_fit();
    }

    // main procedure
    void build_coreset()
    {
        start = std::chrono::system_clock::now();

        // get size
        const unsigned int size = pset.size();

        // coreset size
        const float success_probability = 0.95;
        const unsigned int coreset_size = (double)z / (1.0 - success_probability);
        std::cout << " success probability: " << success_probability << "  ->  coreset size: " << coreset_size << "\n";

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
        for (unsigned int i = 1; i < coreset_size; ++i) {

            std::pair<unsigned int, float> candidate = {0,0};

            // get last center
            point* p = &pset[idx];

            // update (1) dist-min to the intermediate result & (2) candidate
            for (unsigned int j = 0; j < size; ++j) {

                // (1) dist-min update
                const float distance = compute_distance(p, &pset[j]);
                if (dist_min_array[j] > distance) dist_min_array[j] = distance;

                // (2) candidate update
                if (dist_min_array[j] > candidate.second) candidate = {j, dist_min_array[j]};
            }

            // result update
            solution.push_back(candidate.first);
            idx = candidate.first;
        }

        end = std::chrono::system_clock::now();
        running_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        running_time /= 1000;
        //std::cout << " coreset size: " << solution.size() << "\n";
        std::cout << " coreset building done\n";
    }

    // get coreset
    std::vector<unsigned int> get_coreset() { return solution; }

    // get offline time
    double get_offline_time () { return running_time; }

};