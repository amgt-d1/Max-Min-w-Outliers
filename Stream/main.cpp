#include "stream.hpp"


int main()
{
    // file input
    input_parameter();

    // data input
    input_dataset();

    // display current time
    get_current_time();

    // random generator
    std::mt19937 mt(0);
	std::uniform_int_distribution<> rnd(0, 1.0);

    // guessing
    float rad_max = 0;
    for (unsigned int i = 0; i < 5; ++i)
    {
        // make an instance
        gmm g(i);

        // run
        g.diversification();

        float temp = g.get_radius();
        if (temp > rad_max) rad_max = temp;
    }

    // iterate run_num
    for (unsigned int i = 0; i < run_num; ++i)
    {
        // flag for last result
        bool f = 0;
        if (i == run_num - 1) f = 1;

        stream t(i, rad_max);
        t.diversification();
        t.output_file(f);
    }

    return 0;
}