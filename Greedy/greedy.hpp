#include "../utils/data.hpp"
#include <functional>
#include <algorithm>
#include "../utils/vptree.hpp"


double run_avg_time_collect = 0;
double run_avg_time_remove = 0;
double run_avg_time_gmm = 0;


// algorithm
class greedy
{
    // dataset in instance
    std::vector<point> pset;

    // vp-tree
    vp_tree vpt;

    // seed
    unsigned int seed = 0;

    // solution set
    std::vector<unsigned int> solution;

    // vector for dist-min
    std::vector<float> dist_min_array;

    // running time
    double running_time = 0;
    double run_time_collect = 0;
    double run_time_remove = 0;
    double run_time_gmm = 0;

    // radius
    float radius = FLT_MAX;

    // #outliers in centers
    double outlier_num = 0;

    // get #outliers
    void get_stats()
    {
        /*********************/
        /*** get #outliers ***/
        /*********************/
        for (unsigned int i = 0; i < solution.size(); ++i)
        {    
            // get idx
            const unsigned int idx = solution[i];
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

    // 1st phase
    void collect_outliers()
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
            idx = candidate.first;
        }

        end = std::chrono::system_clock::now();
        run_time_collect = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        run_time_collect /= 1000;
    }

    // 2nd phase
    void remove_outliers()
    {
        start = std::chrono::system_clock::now();

        // get size
        const unsigned int size = pset.size();

        // copy solution
        std::vector<unsigned int> solution_temp = solution;

        // init solution
        solution.clear();

        // compute nearest neighbor
        std::vector<std::pair<float, unsigned int>> nearest_neighbor_result;
        for (unsigned int i = 0; i < solution_temp.size(); ++i)
        {
            float dist_min = FLT_MAX;

            // get idx
            const unsigned int idx = solution_temp[i];
            
            for (unsigned int j = 0; j < size; ++j)
            {
                if (j != idx)
                {
                    const float distance = compute_distance(&pset[idx], &pset[j]);
                    if (distance < dist_min) dist_min = distance;
                }
            }

            nearest_neighbor_result.push_back({dist_min, idx});
        }

        end = std::chrono::system_clock::now();
        run_time_remove = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        run_time_remove /= 1000;
    }

    // 2nd phase with vp-tree
    void remove_outliers_with_index()
    {
        start = std::chrono::system_clock::now();

        // copy solution
        std::vector<unsigned int> solution_temp = solution;

        // get size
        const unsigned int size = solution_temp.size();

        // init solution
        solution.clear();

        // top-z outliers
        std::multimap<float, unsigned int, std::greater<float>> result;
		float threshold = 0;

        // compute nearest neighbor
        for (unsigned int i = 0; i < size; ++i)
        {
            // get idx
            const unsigned int idx = solution_temp[i];
            
            std::vector<std::pair<float, unsigned int>> knn_result(2);
            vpt.knn_search(pset[idx].pt, 2, knn_result);

            float distance = knn_result[1].first;
            if (distance > threshold)
            {
                result.insert({distance, idx});
                if (result.size() > z)
				{
					auto it = result.end();
					--it;
					result.erase(it);
				}
				if (result.size() == z)
				{
					auto it = result.end();
					--it;
					threshold = it->first;
				}
            }
        }

        end = std::chrono::system_clock::now();
        run_time_remove = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        run_time_remove /= 1000;
    }

    // 3rd phase
    void GMM()
    {
        // get size
        const unsigned int size = inlier_set.size();

        for (unsigned int i = 0; i < size; ++i) dist_min_array[i] = FLT_MAX;

        start = std::chrono::system_clock::now();

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
        for (unsigned int i = 1; i < k; ++i)
        {
            std::pair<unsigned int, float> candidate = {0,0};

            // get last center
            point* p = &inlier_set[idx];

            // update (1) dist-min to the intermediate result & (2) candidate
            for (unsigned int j = 0; j < size; ++j)
            {
                // (1) dist-min update
                const float distance = compute_distance(p, &inlier_set[j]);
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
        run_time_gmm = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        run_time_gmm /= 1000;
    }

public:

    // constructor
    greedy() {}

    greedy(unsigned int s)
    {
        // init
        seed = s;
        pset = point_set;
        dist_min_array.resize(pset.size());

        const unsigned int size = pset.size();
        for (unsigned int i = 0; i < size; ++i) dist_min_array[i] = FLT_MAX;

        // build vp-tree
        std::vector<std::vector<float>> temp(size);
        for (unsigned int i = 0; i < size; ++i)
        {
            temp[i].resize(dimensionality);
            for (unsigned int j = 0; j < dimensionality; ++j) temp[i][j] = pset[i].pt[j];
        }
        vpt.input(temp, dimensionality);
        vpt.build();
    }

    // destructor
    ~greedy()
    {
        pset.shrink_to_fit();
        solution.shrink_to_fit();
        dist_min_array.shrink_to_fit();
    }

    // main procedure
    void max_min_diversification()
    {
        collect_outliers();
        //remove_outliers();
        remove_outliers_with_index();
        GMM();
    }

    // output result
    void output_file(bool flag)
    {
        std::string f_name = "result/";
        if (dataset_id == 0) f_name += "0_syn/";
        if (dataset_id == 1) f_name += "1_household/";
        if (dataset_id == 2) f_name += "2_covertype/";
        if (dataset_id == 3) f_name += "3_kdd/";
        if (dataset_id == 4) f_name += "4_mirai/";
        
        f_name += "id(" + std::to_string(dataset_id) + ")_k(" + std::to_string(k) + ")_z(" + std::to_string(z) + ")_n(" + std::to_string(cardinality) + ")_greedy.csv";
        std::ofstream file;
        file.open(f_name.c_str(), std::ios::out | std::ios::app);

        if (file.fail())
        {
            std::cerr << " cannot open the output file." << std::endl;
            file.clear();
            return;
        }

        get_stats();
        running_time = run_time_collect + run_time_remove + run_time_gmm;

        file << "run time [msec]: " << running_time
            << ", outlier collect time [msec]: " << run_time_collect
            << ", outlier remove time [msec]: " << run_time_remove
            << ", diversification time [msec]: " << run_time_gmm
            << ", radius: " << radius
            << ", #outliers: " << outlier_num
            << "\n";

        time_avg += running_time;
        radius_avg += radius;
        run_avg_time_collect += run_time_collect;
        run_avg_time_remove += run_time_remove;
        run_avg_time_gmm += run_time_gmm;

        // average result
        if (flag)
        {
            file << "avg. run time [msec]: " << time_avg / run_num
            << ", avg. run time (collect) [msec]: " << run_avg_time_collect / run_num
            << ", avg. run time (remove) [msec]: " << run_avg_time_remove / run_num
            << ", avg. run time (div) [msec]: " << run_avg_time_gmm / run_num
            << ", avg. radius: " << radius_avg / run_num
            << ", avg. #outliers: " << outlier_num_avg / run_num << "\n";
        }

        file.close();
    }
};