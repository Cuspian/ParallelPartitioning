#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <map>
#include <ctime>

using namespace std;
/* Set Partition with partition costs 1 CPU
by Denis Khryashchev*/

typedef vector <int> subset;

int c = 1;
vector <float> costs;
subset min_subset;
float min_sum = 10000;

// Printing a subset or any numeric vector
void print_subset(subset current)
{
    for (int i = 0; i < current.size(); i++)
        cout << current[i] << " ";
    cout << endl;
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

subset get_binary(int n, int p)
{
    subset asubset;
    for (int i = 0; i < n; i++)
        asubset.push_back(0);
    asubset[n - p - 1] = 1;
    return asubset;
}

int bin2int(subset asubset)
{
    int result = 0;
    for (int i = 0; i < asubset.size(); i++)
        result+= asubset[i]*pow(2, i);
    return result;
}

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

subset aggregate(subset current)
{
    int size_ = current.size();
    current[size_-1]++;
    for (int i = size_ - 1; i >= 1; i--)
        {
            if (current[i] == *max_element(begin(current), end(current) - size_ + i ) + 2)
            {
                current[i] = 1;
                current[i-1]++;
            }
        }
    return current;
}

subset next_set(subset current)
{
    int size_ = current.size();
    current[size_ -2]++;
    current[size_ -1] = 1;
    for (int i = size_ - 1; i >= 1; i--)
    {
        if (current[i] == *max_element(begin(current), end(current) - size_ + i ) + 2)
        {
            current[i] = 1;
            current[i-1]++;
        }
    }
    return current;
}

void set_between(subset asubset, subset bsubset)
{
    while (asubset != bsubset)
    {
        //cout << c << ". ";
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
        asubset = aggregate(asubset);
        c++;
    }
}

int main()
{
    float start_tick = clock();
    // Reading the costs of partitions
    ifstream input_file("partition_costs.txt");
    float cost;
    while (input_file >> cost)
        costs.push_back(cost);

    // Number of elements
    int n = (int)log2(costs.size() + 1);

    // Initial partition
    subset asubset;
    for (int i = 0; i < n; i++)
        asubset.push_back(1);

    while (asubset != next_set(end_set(n)))
    {
        subset bsubset = next_set(asubset);
        set_between(asubset, bsubset);
        asubset = bsubset;
    }

    cout << endl << "Total number of subsets (partitions) is " << Bell(n) << endl;
    cout << "Minimal cost is " << min_sum << endl;
    cout << "Corresponding subset is " << endl;
    print_subset(min_subset);

    float end_tick = clock();
    cout << "It took " + to_string((end_tick - start_tick) / double(CLOCKS_PER_SEC)*1000) + " ms" << endl;
    cout << "With the use of 1 CPU" << endl;
}
