#include "gmm.hpp"
#include <cfloat>
#include <cmath>


// error factor
const float eps = 0.01;


class stream
{
    // dataset in instance
    std::vector<point> pset;

    // seed
    unsigned int seed = 0;

    // solution set
    std::vector<unsigned int> solution;
    std::vector<unsigned int> solution_fix;

    // vector for dist-min
    std::vector<float> dist_min_array;

    // running time
    double running_time = 0;

    // radius
    float radius = FLT_MAX;
    float radius_guess = 0;
    float exponent = 35.0;
    float exponent_min = 0.0;   // fixed by eps
    float exponent_max = 69.0;  // fixed by eps

    float radius_fix = 0;

    // #outliers in centers
    double outlier_num = 0;

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

public:

    stream() {}

    stream(const unsigned int s, const float guess)
    {
        // init
        seed = s;
        pset = point_set;
        dist_min_array.resize(pset.size());
        radius_guess = guess;

        const unsigned int size = pset.size();
        for (unsigned int i = 0; i < size; ++i) dist_min_array[i] = FLT_MAX;
    }

    // destructor
    ~stream()
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

        while (1)
        {
            const float rad = radius_guess * pow((1.0 + eps), exponent);
            ++iteration;

            /******************/
            /*** 1st sample ***/
            /******************/
            unsigned int idx = rnd_int(mt);
            solution.push_back(idx);

            for (unsigned int i = 0; i < size; ++i)
            {
                dist_min_array[i] = FLT_MAX;

                for (unsigned int j = 0; j < solution.size(); ++j)
                {
                    // distance computation
                    const float distance = compute_distance(&pset[i], &pset[solution[j]]);

                    if (dist_min_array[i] > distance) dist_min_array[i] = distance;
                }

                if (dist_min_array[i] >= rad / 2.0 && dist_min_array[i] <= rad * 1.0)
                {
                    solution.push_back(i);
                    if (radius > dist_min_array[i]) radius = dist_min_array[i];    
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
            else
            {
                init(size);
            }
        }

        end = std::chrono::system_clock::now();
        running_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        running_time /= 1000;
    }

    // output result
    void output_file(bool flag)
    {
        std::string f_name = "result/";
        if (dataset_id == 1) f_name += "1_household/";
        if (dataset_id == 2) f_name += "2_covertype/";
        if (dataset_id == 3) f_name += "3_kdd/";
        if (dataset_id == 4) f_name += "4_mirai/";
        
        f_name += "id(" + std::to_string(dataset_id) + ")_k(" + std::to_string(k) + ")_z(" + std::to_string(z) + ")_n(" + std::to_string(cardinality) + ")_stream.csv";
        std::ofstream file;
        file.open(f_name.c_str(), std::ios::out | std::ios::app);

        if (file.fail())
        {
            std::cerr << " cannot open the output file." << std::endl;
            file.clear();
            return;
        }

        get_stats();

        file << "run time [msec]: " << running_time << ", radius: " << radius_fix << ", size: " << solution_fix.size() << ", #outliers: " << outlier_num << "\n";

        time_avg += running_time;
        radius_avg += radius_fix;

        // average result
        if (flag)
        {
            file << "avg. run time [msec]: " << time_avg / run_num
                << ", avg. radius: " << radius_avg / run_num
                << ", avg. #outliers: " << outlier_num_avg / run_num
                << "\n";
        }
        file.close();
    }
};
