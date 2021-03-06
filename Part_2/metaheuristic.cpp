/***************************************************************************
    metaheuristic.cpp 
    (C) 2021 by C. Blum & M. Blesa
    
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Timer.h"
#include "Random.h"
#include "../Part_1/greedy_class.cpp"
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <climits>
#include <iomanip>
#include <float.h>

// global variables concerning the random number generator (in case needed)
time_t t;
Random* rnd;

// Data structures for the problem data
struct hash_pair {
    template <class T1, class T2>
    size_t operator()(const pair<T1, T2>& p) const
    {
        auto hash1 = hash<T1>{}(p.first);
        auto hash2 = hash<T2>{}(p.second);
        return hash1 ^ hash2;
    }
};

int n_of_nodes;
int n_of_arcs;
vector< unordered_set<int> > neighbors;
unordered_map< pair<int,double>, long, hash_pair > tabuAdd; // <(i,j), k> means that add the node i to solution with score j is tabu for k steps
unordered_map< pair<int,double>, long, hash_pair > tabuDelete; // <i, k> means that delete the node i to solution with score j is tabu for k steps
unordered_set<int> globalMinimum;
double scoreGlobalMinimum;
vector<int> neighbors_popularity;
double percentage;


// string for keeping the name of the input file
string inputFile;

// computing time limit for each application of the metaheuristic
double time_limit = 600.0;

// number of applications of the metaheuristic
int n_apps = 1;

// dummy parameters as examples for creating command line parameters 
// (see function read_parameters(...))
int dummy_integer_parameter = 0;
int dummy_double_parameter = 0.0;


inline int stoi(string &s) {

  return atoi(s.c_str());
}

inline double stof(string &s) {

  return atof(s.c_str());
}

void read_parameters(int argc, char **argv) {

    int iarg = 1;
    while (iarg < argc) {
        if (strcmp(argv[iarg],"-i") == 0) inputFile = argv[++iarg];
        // reading the computation time limit 
        // from the command line (if provided)
        else if (strcmp(argv[iarg],"-t") == 0) time_limit = atoi(argv[++iarg]); 
        // reading the number of applications of the metaheuristic 
        // from the command line (if provided)
        else if (strcmp(argv[iarg],"-n_apps") == 0) n_apps = atoi(argv[++iarg]); 
        // example for creating a command line parameter 
        // param1 -> integer value is stored in dummy_integer_parameter
        else if (strcmp(argv[iarg],"-param1") == 0) {
            dummy_integer_parameter = atoi(argv[++iarg]); 
        }
        // example for creating a command line parameter 
        // param2 -> double value is stored in dummy_double_parameter
        else if (strcmp(argv[iarg],"-param2") == 0) {
            dummy_double_parameter = atof(argv[++iarg]);
        }
        iarg++;
    }
}


//////////////////////////////////////////////////////////////
//                   HELPER FUNCTIONS                       //
//////////////////////////////////////////////////////////////


//Compute the percentage of neighbors in the solution
void compute_percentage_neighbors(const unordered_set<int> &solution) {
    percentage = 0.0;
    for (unordered_set<int> ns : neighbors) {
        double percentage_node = 0.0;
        for (int neighbor : ns) {
            if (solution.find(neighbor) != solution.end()) {
                percentage_node++;
            }
        }
        if (ns.size() > 0.0) {
            percentage += percentage_node / ns.size();
        }
    }
    scoreGlobalMinimum = percentage;
}


//Return if a node can be deleted in the solution
bool canDelete(int node, unordered_set<int> &solution) {
    for (int neighbor : neighbors[node]) {
        if ((neighbors_popularity[neighbor] - 1.f) < neighbors[neighbor].size()/2.f) {
            return false;
        }
    }
    return true;
}


//////////////////////////////////////////////////////////////
//                      HEURISTIC                           //
//////////////////////////////////////////////////////////////


//Heuristic used to find the best solution
double computeHeuristic(const unordered_set<int>& solution, int score) {
    return 1000*solution.size() + score;
}

//////////////////////////////////////////////////////////////
//                      OPERATORS                           //
//////////////////////////////////////////////////////////////

//Operator to add a nodes
double addNode(unordered_set<int>& solution, int node) {
    solution.insert(node);
    double percentage_aux = percentage;
    for (int neighbor : neighbors[node]) {
        neighbors_popularity[neighbor]++;
        percentage_aux += (1.0 / neighbors[neighbor].size());
    }
    return percentage_aux;
}

//Operetor to delete a node
double deleteNode(unordered_set<int>& solution, int node) {
    solution.erase(node);
    double percentage_aux = percentage;
    for (int neighbor : neighbors[node]) {
        neighbors_popularity[neighbor]--;
        percentage_aux -= (1.0 / neighbors[neighbor].size());
    }
    return percentage_aux;
}


//////////////////////////////////////////////////////////////
//                    TABU SERACH                           //
//////////////////////////////////////////////////////////////

//Tabu Search
void tabuSearch(unordered_set<int> solution, Timer timer) {
    int it = 0;
    long itInTabu = neighbors.size();
    double scoreLocalMinimum;
    unordered_set<int> localBestSolution;
    for (int i = 0; i < neighbors.size(); i++) 
        localBestSolution.insert(i);

    vector<int> neighbors_popularity_min = neighbors_popularity;

    while (timer.elapsed_time(Timer::VIRTUAL) <= time_limit) {
        int nd = -1;
        bool add = false;
        bool del = false;

        scoreLocalMinimum = DBL_MAX-neighbors.size();
        //Loop to add nodes
        for (int node = 0; node < neighbors.size(); node++) {
            if (solution.find(node) == solution.end()) {
                double percentage_aux = addNode(solution, node);
                double currHeuristicVal = computeHeuristic(solution,percentage_aux);
                if (tabuAdd[{node, 0}] <= it || currHeuristicVal < computeHeuristic(globalMinimum,scoreGlobalMinimum)) {
                    if (currHeuristicVal < computeHeuristic(localBestSolution,scoreLocalMinimum)) {
                        scoreLocalMinimum = percentage_aux;
                        localBestSolution = solution;
                        neighbors_popularity_min = neighbors_popularity;
                        add = true;
                        del = false;
                        nd = node;
                    }
                }
                for (int neighbor : neighbors[node]) {
                    neighbors_popularity[neighbor]--;
                }
                solution.erase(node);
            }
        }

        //Loop to delete nodes
        for (int node = 0; node < neighbors.size(); node++) {
            if (solution.find(node) != solution.end()) { 
                if (canDelete(node, solution)) {
                    double percentage_aux = deleteNode(solution, node);
                    double currHeuristicVal = computeHeuristic(solution,percentage_aux);
                    
                    if (tabuAdd[{node, percentage_aux}] <= it or currHeuristicVal < computeHeuristic(globalMinimum,scoreGlobalMinimum)) {
                        if (computeHeuristic(solution,percentage_aux) < computeHeuristic(localBestSolution,scoreLocalMinimum)) {
                            scoreLocalMinimum = percentage_aux;
                            localBestSolution = solution;
                            neighbors_popularity_min = neighbors_popularity;
                            del = true;
                            add = false;
                            nd = node;
                        }   
                    }
                    for (int neighbor : neighbors[node]) {
                        neighbors_popularity[neighbor]++;
                    }
                    solution.insert(node);
                }
            }
        }

        if (LONG_MAX - itInTabu <= (long) it) {
            tabuAdd.clear();
            tabuDelete.clear();
            it = 0;
            cout << "Big num" << endl;
        }

        if (computeHeuristic(solution,scoreLocalMinimum) < computeHeuristic(globalMinimum,scoreGlobalMinimum)) {
            scoreGlobalMinimum = scoreLocalMinimum;
            globalMinimum = localBestSolution;
            neighbors_popularity = neighbors_popularity_min;
            cout << "New global minimum: " << globalMinimum.size()<< endl;
        }

        if (nd != -1) {
            solution = localBestSolution;
            percentage = scoreLocalMinimum;
            neighbors_popularity = neighbors_popularity_min;
            if (del) tabuAdd[{nd, 0}] = itInTabu + it;
            
            else if (add) tabuAdd[{nd, 0}] = itInTabu + it;
        }
        it++;
    }

    for (pair <pair <int, int>, long> px : tabuAdd){
        cout << px.first.first << " " << px.first.second << ": " << px.second << endl;
    }
}

/**********
Main function
**********/

int main( int argc, char **argv ) {

    read_parameters(argc,argv);
    
    // setting the output format for doubles to 2 decimals after the comma
    std::cout << std::setprecision(2) << std::fixed;

    // initializing the random number generator. A random number 
    // between 0 and 1 is obtained with: double rnum = rnd->next();
    rnd = new Random((unsigned) time(&t));
    rnd->next();

    // vectors for storing the result and the computation time 
    // obtained by the <n_apps> applications of the metaheuristic
    vector<double> results(n_apps, std::numeric_limits<int>::max());
    vector<double> times(n_apps, 0.0);

    // opening the corresponding input file and reading the problem data
    ifstream indata;
    indata.open(inputFile.c_str());
    if (not indata) { // file couldn't be opened
        cout << "Error: file could not be opened" << endl;
    }

    indata >> n_of_nodes;
    indata >> n_of_arcs;
    neighbors = vector< unordered_set<int> >(n_of_nodes);
    int u, v;
    while (indata >> u >> v) {
        neighbors[u - 1].insert(v - 1);
        neighbors[v - 1].insert(u - 1);
    }
    indata.close();

    // main loop over all applications of the metaheuristic
    for (int na = 0; na < n_apps; ++na) {

        // the computation time starts now
        Timer timer;

        // Example for requesting the elapsed computation time at any moment: 
        // double ct = timer.elapsed_time(Timer::VIRTUAL);

        cout << "start application " << na + 1 << endl;

        // HERE GOES YOUR METAHEURISTIC

        setNeighbor (neighbors);
        unordered_set <int> sAux = greedy();
        sAux = remove_nodes(sAux);
        globalMinimum = sAux;
        neighbors_popularity = getNeighborPopularity();
        compute_percentage_neighbors(sAux);
        cout << "Nodes greedy: " << sAux.size() << endl;

        tabuSearch(sAux, timer);

        cout << "Number of nodes: " << globalMinimum.size() << endl;

        if (check_PIDS(globalMinimum)) cout << "YEEEEEES" << endl;

        // For implementing the metaheuristic you probably want to take profit 
        // from the greedy heuristic and/or the local search method that you 
        // already developed.
        //
        // Whenever the best found solution is improved, first take the 
        // computation time as explained above. Say you store it in variable ct.
        // Then, write the following to the screen: 
        // cout << "value " << <value of the new best found solution>;
        // cout << "\ttime " << ct << endl;
        //
        // Store the value of the new best found solution in vector results: 
        // results[na] = <value of the new best found solution>;
        //
        // And store the current computation time (that is, the time measured 
        // at that moment and stored in variable "ct") in vector times: 
        // times[na] = ct;
        //
        // Stop the execution of the metaheuristic 
        // once the time limit "time_limit" is reached.

        cout << "end application " << na + 1 << endl;
    }

    // calculating the average of the results and computation times, 
    // and their standard deviations, and write them to the screen
    double r_mean = 0.0;
    int r_best = std::numeric_limits<int>::max();
    double t_mean = 0.0;
    for (int i = 0; i < int(results.size()); i++) {
        r_mean = r_mean + results[i];
        if (int(results[i]) < r_best) r_best = int(results[i]);
        t_mean = t_mean + times[i];
    }
    r_mean = r_mean/double(results.size());
    t_mean = t_mean/double(times.size());
    double rsd = 0.0;
    double tsd = 0.0;
    for (int i = 0; i < int(results.size()); i++) {
        rsd = rsd + pow(results[i]-r_mean,2.0);
        tsd = tsd + pow(times[i]-t_mean,2.0);
    }
    rsd = rsd/double(results.size());
    if (rsd > 0.0) {
        rsd = sqrt(rsd);
    }
    tsd = tsd/double(results.size());
    if (tsd > 0.0) {
        tsd = sqrt(tsd);
    }
    // printing statistical information
    cout << r_best << "\t" << r_mean << "\t" << rsd << "\t";
    cout << t_mean << "\t" << tsd << endl;
}

