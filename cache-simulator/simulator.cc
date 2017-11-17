#include "common.h"
#include "readFile.h"
#include "cache.h"
#include <cmath>
#include <iomanip>
#include <sstream>

int main(int argc, char *argv[])  {   
    
    //int block_size = 32;
    // int l1_size = 2048;
    // int l1_associate = 8;
    // unsigned repl_policy = 0;

    // int l2_size = 0;
    // int l2_associate = 0;
    // unsigned inclusion = 0;
    
    //string trace_file = "trace_file/perl_trace.txt";

    int block_size;
    int l1_size;
    int l1_associate;
    int repl_policy;

    int l2_size;
    int l2_associate;
    int inclusion;
    string trace_file;

    vector<int*> inputs;
    inputs.push_back(&block_size);
    inputs.push_back(&l1_size);
    inputs.push_back(&l1_associate);
    inputs.push_back(&l2_size);
    inputs.push_back(&l2_associate);
    inputs.push_back(&repl_policy);
    inputs.push_back(&inclusion);

    for (unsigned i = 0; i < argc; i++)
    {   
        if (i < inputs.size())
        {
            istringstream iss(argv[i+1]);
            if (iss >> *inputs[i])
            {
                //cout << "input success" << endl;
            }
        }
    }


    istringstream iss_string(argv[8]);
    if (iss_string >> trace_file)
    {
        //cout << trace_file << endl;
    }

    vector<string> memory_trace = readTraceFile(trace_file);

    vector<string>::iterator trace_iterator;


    Cache cache_l1(block_size, l1_size, l1_associate, repl_policy); // for validation#0

    for (trace_iterator = memory_trace.begin(); trace_iterator != memory_trace.end(); trace_iterator++) {
        cache_l1.fetchTag(*trace_iterator);
    }
    cout << setprecision(4) << fixed;

    cout << "============ Simulator configuration ============" << endl;
    cout << "BLOCKSIZE:                         " << block_size << endl;
    cout << "L1_SIZE:                           " << l1_size << endl;
    cout << "L1_ASSOC:                          " << l1_associate << endl;
    cout << "L2_SIZE:                           " << l2_size << endl;
    cout << "L2_ASSOC:                          " << l2_associate << endl;
    cout << "REPLACEMENT POLICY:                " << repl_policy << endl;
    cout << "INCLUSION POLICY:                  " << inclusion << endl;
    cout << "TRACE FILE:                        " << trace_file << endl;
    cout << "============    Simulation results   ============" << endl;
    cout << "a. number of L1 reads:             " << cache_l1.read_count << endl;
    cout << "b. number of L1 read misses:       " << cache_l1.read_miss << endl;
    cout << "c. number of L1 writes:            " << cache_l1.write_count << endl;
    cout << "d. number of L1 write misses:      " << cache_l1.write_miss << endl;
    cout << "e. L1 miss rate:                   " << round((float)(cache_l1.read_miss + cache_l1.write_miss) / (float)(cache_l1.read_count + cache_l1.write_count)*10000.0) / 10000.0 << endl;
    cout << "f. number of L1->L2 writebacks:    " << cache_l1.write_back << endl;
    cout << "g. number of L2 reads:             " << 0 << endl;
    cout << "h. number of L2 read misses:       " << 0 << endl;
    cout << "i. number of L2 writes:            " << 0 << endl;
    cout << "j. number of L2 write misses:      " << 0 << endl;
    cout << "k. L2 miss rate:                   " << 0.0000 << endl;
    cout << "l. number of L2->Mem writebacks:   " << 0 << endl;
    cout << "m. total memory traffic:           " << cache_l1.read_miss + cache_l1.write_miss + cache_l1.write_back<< endl;

    return 0;
}




















