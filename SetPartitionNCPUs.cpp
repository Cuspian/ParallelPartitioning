#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <map>
#include <ctime>

using namespace std;
/* Set Partition with partition costs 1 CPU
by Denis Khryashchev*/

// Type to store subsets
typedef vector <int> subset;

// Vector of costs of subsets
vector <float> costs;
// The optimal partition
subset min_subset;
// Cost of the optimal partition
float min_sum = 10000;

// Printing a subset or any numeric vector
void print_subset(subset current)
{
    cout << "{";
    for (int i = 0; i < current.size() - 1; i++)
        cout << current[i] << ", ";
    cout << current[current.size() - 1] << "}";
}

// Final subset, e.g. {1 2 3 3}
subset end_set(int n)
{
    subset asubset;
    for (int i = 1; i <= n - 1; i++)
        asubset.push_back(i);
    asubset.push_back(n-1);
    return asubset;
}

// Construct binary sequence of size n with all 0 except for p
subset get_binary(int n, int p)
{
    subset asubset;
    for (int i = 0; i < n; i++)
        asubset.push_back(0);
    asubset[n - p - 1] = 1;
    return asubset;
}

// Turn binary sequence to an integer
int bin2int(subset asubset)
{
    int result = 0;
    for (int i = 0; i < asubset.size(); i++)
        result+= asubset[i]*pow(2, i);
    return result;
}

// Turn the enumerated subsets into a partition through conversion to binary sequences
subset map_subsets(subset asubset)
{
    int size_ = asubset.size();
    map<int, subset> dict;
    map<int, subset>::iterator it;
    for (int i = 0; i < size_; i++)
    {
        subset binary = get_binary(size_, i);
        if ((it = dict.find(asubset[i])) != dict.end())
           transform(dict[asubset[i]].begin(), dict[asubset[i]].end(), binary.begin(), dict[asubset[i]].begin(), plus<int>());
        else
            dict[asubset[i]] = binary;
    }
    subset result;
    for (it = dict.begin(); it != dict.end(); ++it)
        result.push_back(bin2int(it -> second));
    return result;
}

// Calculating the total number of partitions, Bell number through the triangle
long long Bell(int n)
{
    if (n <= 1)
        return 1;
    subset temp;
    temp.push_back(1);
    temp.push_back(1);
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j <= i; j++)
            temp.push_back(temp[temp.size() - 1] + temp[temp.size() - i - 1]);
    return temp[temp.size()-1];
}

// Move on to the nearest partition
subset aggregate(subset current)
{
    int size_ = current.size();
    current[size_-1]++;
    for (int i = size_ - 1; i >= 1; i--)
    {
        if (current[i] == *max_element(current.begin(), current.end() - size_ + i ) + 2)
        {
            current[i] = 1;
            current[i-1]++;
        }
    }
    return current;
}

// Jump to the next partition group for parallelizing
subset next_set(subset current)
{
    int size_ = current.size();
    current[size_ -2]++;
    current[size_ -1] = 1;
    for (int i = size_ - 1; i >= 1; i--)
    {
        if (current[i] == *max_element(current.begin(), current.end() - size_ + i ) + 2)
        {
            current[i] = 1;
            current[i-1]++;
        }
    }
    return current;
}

// Evaluate costs of partitions of a group between asubset and bsubset
void set_between(subset asubset, subset bsubset)
{
    while (asubset != bsubset)
    {

        subset mapped = map_subsets(asubset);
        float sum_ = 0.0;
        for (int i = 0; i < mapped.size(); i++)
            sum_+= costs[mapped[i] - 1];

        if (min_sum > sum_)
        {
            min_sum = sum_;
            min_subset = mapped;
        }

        //print_subset(mapped);
        //cout << " costs " << sum_ << endl;
        asubset = aggregate(asubset);
    }
}

int main(int argc, char** argv) {

    float start_tick = clock();
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Reading the costs of partitions
    ifstream input_file("partition_costs.txt");
    float cost;
    while (input_file >> cost)
        costs.push_back(cost);

    // Number of elements
    int n = (int)log2(costs.size() + 1);

    // Main CPU
    if (rank == 0)
    {
        // Initial partition
        subset asubset;
        for (int i = 0; i < n; i++)
            asubset.push_back(1);

        int current = 1;
        while (asubset != next_set(end_set(n)))
        {

            subset bsubset = next_set(asubset);

            MPI_Send(&asubset[0], asubset.size(), MPI_INT, current, 0, MPI_COMM_WORLD);
            MPI_Send(&bsubset[0], bsubset.size(), MPI_INT, current, 0, MPI_COMM_WORLD);

            //set_between(asubset, bsubset);
            asubset = bsubset;
            current++;
            if (current == world_size)
                current = 1;
        }

        asubset[0] = -1;
        for (int i = 1; i < world_size; i++)
            MPI_Send(&asubset[0], asubset.size(), MPI_INT, i, 0, MPI_COMM_WORLD);

        for (int i = 1; i < world_size; i++)
        {
            float curr_sum;
            MPI_Recv(&curr_sum, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            subset temp;
            int received = 0;
            while (1 == 1)
            {
                MPI_Recv(&received, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if (received == -1)
                    break;
                temp.push_back(received);
            }

            if (curr_sum < min_sum)
            {
                min_sum = curr_sum;
                min_subset = temp;
            }

        }

        cout << endl << " ============================= SOLUTION =============================" << endl;
        cout << "Total number of subsets (partitions) is " << Bell(n) << endl;
        cout << "Minimal cost is " << min_sum << endl;
        cout << "Corresponding subset is ";
        print_subset(min_subset);
        cout << endl;

        float end_tick = clock();
        end_tick = (end_tick - start_tick) / double(CLOCKS_PER_SEC)*1000;
        cout << "It took " << end_tick << " ms" << endl;
        cout << "With the use of " << world_size << " CPUs" << endl;
    }

    // Subordinate CPUs
    if (rank > 0)
    {
        subset aset;
        subset bset;
        aset.resize(n);
        bset.resize(n);

        while (1 == 1)
        {
            MPI_Recv(&aset[0], n, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (aset[0] == -1)
                break;
            MPI_Recv(&bset[0], n, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            set_between(aset, bset);
        }

        MPI_Send(&min_sum, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        min_subset.push_back(-1);
        for (int i = 0; i < min_subset.size(); i++)
            MPI_Send(&min_subset[i], 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    // Finalize the MPI environment.
    MPI_Finalize();

    return 0;
}
