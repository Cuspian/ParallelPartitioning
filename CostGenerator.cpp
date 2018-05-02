#include <cstdlib>
#include <math.h>
#include <iostream>
#include <fstream>

using namespace std;
/* Random Cost generator for Set Partitioning Problem
    by Denis Khryashchev
    Input argument sets the N number of elements for which
    the 2^N - 1 partitions costs will be generated in (0, 1)
*/

int main(int argc, char **argv)
{
    ofstream output_file;
    int n_costs = pow(2, 4) - 1;
    if (argc > 1) n_costs = pow(2, stoi(argv[1])) - 1;
    cout << "Generating " << n_costs << " partitioning costs" << endl;
    output_file.open ("partition_costs.txt");
    for (int i = 0; i < n_costs; i++)
    {
        float cost = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        output_file << cost << endl;
    }
    output_file.close();
    cout << "Saving to partition_costs.txt" << endl;
}
