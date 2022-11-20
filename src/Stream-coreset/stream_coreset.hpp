#include "gmm.hpp"
#include <cfloat>
#include <cmath>


double run_avg_time_coreset = 0;


class stream_coreset
{
    // dataset in instance
    std::vector<point> pset;

    // coreset
    std::vector<unsigned int> coreset;

    // seed
    unsigned int seed = 0;

    // solution set
    std::vector<unsigned int> solution;
    std::vector<unsigned int> solution_fix;

    // vector for dist-min
    std::vector<float> dist_min_array;

    // running time
    double running_time = 0;
    double running_time_coreset = 0;

    // radius
    float radius = FLT_MAX;
    float radius_guess = 0;

    // error factor
    const float eps = 0.01;
    float exponent = 35.0;
    float exponent_min = 1.0;   // fixed by eps
    float exponent_max = 69.0;  // fixed by eps

    float radius_fix = 0;

    // #outliers in centers
    double outlier_num = 0;

    // #iterations via guess
    unsigned int iteration = 0;


    // get result stats
    void get_stats()
    {
        /*********************/
        /*** get #outliers ***/
        /*********************/
        for (unsigned int i = 0; i < k; ++i)
        {            
            // get idx
            const unsigned int idx = solution_fix[i];
            if (pset[idx].in == 0) ++outlier_num;
        }

        // aggregation
        outlier_num_avg += outlier_num;
    }
    
    // distance computation
    float compute_distance(const point *l, const point *r)
    {
        float distance = 0;
        for (unsigned int i = 0; i < dimensionality; ++i) distance += (l->pt[i] - r->pt[i]) * (l->pt[i] - r->pt[i]);
        return sqrt(distance);
    }

    // init
    void init(const unsigned int size)
    {
        radius = FLT_MAX;
        solution.clear();
        for (unsigned int i = 0; i < size; ++i) dist_min_array[i] = FLT_MAX;
    }

    // coreset construction
    void build_coreset()
    {
        start = std::chrono::system_clock::now();

        // get size
        const unsigned int size = pset.size();

        // coreset size
        const float success_probability = 0.90;
        const unsigned int coreset_size = (float)z / (1.0 - success_probability);
        std::cout << " coreset size: " << coreset_size << "\n";

        // random generator
        std::mt19937 mt(seed);
        std::uniform_int_distribution<> rnd_int(0, size - 1);

        /******************/
        /*** 1st sample ***/
        /******************/
        unsigned int idx = rnd_int(mt);
        coreset.push_back(idx);

        /**********************/
        /*** i-th iteration ***/
        /**********************/
        for (unsigned int i = 1; i < coreset_size; ++i)
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

            // coreset update
            coreset.push_back(candidate.first);
            idx = candidate.first;
        }

        end = std::chrono::system_clock::now();
        running_time_coreset = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        running_time_coreset /= 1000;
        std::cout << " coreset building done\n";
    }

    // Max-Min Diversification
    void diversification()
    {
        start = std::chrono::system_clock::now();

        // get size
        const unsigned int size = coreset.size();

        // random generator
        std::mt19937 mt(seed);
        std::uniform_int_distribution<> rnd_int(0, size - 1);

        while (1)
        {
            const float rad = radius_guess * pow((1.0 + eps), exponent);
            ++iteration;

            /******************/
            /*** 1st sample ***/
            /******************/
            unsigned int idx = rnd_int(mt);
            solution.push_back(coreset[idx]);

            for (unsigned int i = 0; i < size; ++i)
            {
                // get idx
                idx = coreset[i];

                // init
                dist_min_array[idx] = FLT_MAX;

                for (unsigned int j = 0; j < solution.size(); ++j)
                {
                    // get idx
                    const unsigned int idx_ = solution[j];

                    // distance computation
                    const float distance = compute_distance(&pset[idx], &pset[idx_]);

                    if (dist_min_array[idx] > distance) dist_min_array[idx] = distance;
                }

                if (dist_min_array[idx] >= rad / 2.0 && dist_min_array[idx] <= rad * 1.0)
                {
                    solution.push_back(idx);
                    if (radius > dist_min_array[idx]) radius = dist_min_array[idx];    
                }
                if (solution.size() == k) break;
            }

            // binary search
            if (solution.size() == k)
            {
                if (radius_fix < radius)
                {
                    radius_fix = radius;
                    solution_fix = solution;
                }
                if (exponent == exponent_min) break;
                exponent_min = exponent;
                exponent = std::ceil((exponent + exponent_max) / 2.0);
            }
            else
            {
                if (exponent == exponent_max) break;
                exponent_max = exponent;
                exponent = std::ceil((exponent + exponent_min) / 2.0);
            }

            // continue or terminate
            if (exponent_min == exponent_max)
            {
                break;
            }
            else {
                init(size);
            }
        }

        end = std::chrono::system_clock::now();
        running_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        running_time /= 1000;
    }

public:

    // constructor
    stream_coreset() {}

    // constructor
    stream_coreset(const unsigned int s, const float guess)
    {
        // init
        seed = s;
        pset = point_set;
        dist_min_array.resize(pset.size());
        radius_guess = guess;

        const unsigned int size = pset.size();
        for (unsigned int i = 0; i < size; ++i) dist_min_array[i] = FLT_MAX;
    }

    // constructor
    stream_coreset(const unsigned int s, const float guess, const std::vector<unsigned int> &_coreset, const double time_coreset)
    {
        // init
        seed = s;
        pset = point_set;
        dist_min_array.resize(pset.size());
        radius_guess = guess;
        coreset = _coreset;
        running_time_coreset = time_coreset;

        const unsigned int size = pset.size();
        for (unsigned int i = 0; i < size; ++i) dist_min_array[i] = FLT_MAX;
    }

    // destructor
    ~stream_coreset()
    {
        pset.shrink_to_fit();
        solution.shrink_to_fit();
        dist_min_array.shrink_to_fit();
        coreset.shrink_to_fit();
    }

    // overall operation
    void run()
    {
        // coreset construction
        if (coreset.size() == 0) build_coreset();

        // max-min diversification
        diversification();
    }

    // output result
    void output_file(bool flag)
    {
        std::string f_name = "result/";
        if (dataset_id == 1) f_name += "1_household/";
        if (dataset_id == 2) f_name += "2_covertype/";
        if (dataset_id == 3) f_name += "3_kdd/";
        if (dataset_id == 4) f_name += "4_mirai/";
        
        f_name += "id(" + std::to_string(dataset_id) + ")_k(" + std::to_string(k) + ")_z(" + std::to_string(z) + ")_n(" + std::to_string(cardinality) + ")_stream-coreset.csv";
        std::ofstream file;
        file.open(f_name.c_str(), std::ios::out | std::ios::app);

        if (file.fail())
        {
            std::cerr << " cannot open the output file." << std::endl;
            file.clear();
            return;
        }

        get_stats();

        file << "coreset build time [msec]: " << running_time_coreset << ", run time [msec]: " << running_time << ", radius: " << radius_fix << ", size: " << solution_fix.size() << ", #outliers: " << outlier_num << "\n";

        time_avg += running_time;
        radius_avg += radius_fix;
        run_avg_time_coreset += running_time_coreset;

        // average result
        if (flag)
        {
            file << "avg. run time (coreset) [msec]: " << run_avg_time_coreset / run_num
                << ", avg. run time [msec]: " << time_avg / run_num
                << ", avg. radius: " << radius_avg / run_num
                << ", avg. #outliers: " << outlier_num_avg / run_num
                << "\n";
        }
        file.close();
    }
};
