#include "common.h"
#include "readFile.h"

#include "bimodal.h"
#include "gshare.h"
#include "hybrid.h"

#include "btb.h"

#include <iomanip>
#include <sstream>


/*
Entrance for different types of predictors: bimodal; gshare; hybrid
Different types of predictors has different number of command line argument
Feed instruction one by one to the predictor. After iterating thought the whole file, print the results
*/

void bimodal(int argc, char *argv[])
{
    int i_gg;
    int i_btb;
    int assoc_btb;
    string trace_file;

    vector<int*> inputs;
    inputs.push_back(&i_gg);
    inputs.push_back(&i_btb);
    inputs.push_back(&assoc_btb);

    for (unsigned i = 0; i < inputs.size(); i++)
    {   
        istringstream iss(argv[i+2]);
        iss >> *inputs[i];
    }

    istringstream iss_string(argv[5]);
    iss_string >> trace_file;

    // initialize an instance of Bimodal predictor
    Bimodal bimodal_predictor(i_gg);

    // read trace file and start predicting
    vector<string> memory_trace = readTraceFile(trace_file);
    vector<string>::iterator trace_iterator;
    for (trace_iterator = memory_trace.begin(); trace_iterator != memory_trace.end(); trace_iterator++) 
    {
        bimodal_predictor.prediction(*trace_iterator);
        bimodal_predictor.updateCounter();
    }

    cout << "Command Line:" << endl;
    cout << "./sim_bp " << "bimodal " <<  i_gg << " " <<  i_btb << " " << assoc_btb << " " << trace_file << endl;
    cout << endl;

    cout << "Final Bimodal Table Contents: " << endl;
    for (unsigned i = 0; i < bimodal_predictor.data_vec.size(); i++)
    {
        cout << "table[" << i << "]: " << bimodal_predictor.data_vec[i] << endl;
    };
    cout << endl;

    cout << "Final Branch Predictor Statistics: " << endl;
    cout << "a. Number of branches: " << bimodal_predictor.total_prediction << endl;
    cout << "b. Number of predictions from the branch predictor: " << bimodal_predictor.total_prediction << endl;
    cout << "c. Number of mispredictions from the branch predictor: " << bimodal_predictor.misprediction << endl;
    cout << "d. Number of mispredictions from the BTB: " << 0 << endl;
    cout << setprecision(2) << fixed;
    cout << "e. Misprediction Rate: " << (double)bimodal_predictor.misprediction / (double)bimodal_predictor.total_prediction * 100<< " percent" << endl;
}


void gshare(int argc, char *argv[])
{
    int i_gg;
    int i_hh;
    int i_btb;
    int assoc_btb;
    string trace_file;

    vector<int*> inputs;
    inputs.push_back(&i_gg);
    inputs.push_back(&i_hh);
    inputs.push_back(&i_btb);
    inputs.push_back(&assoc_btb);

    for (unsigned i = 0; i < inputs.size(); i++)
    {   
        istringstream iss(argv[i+2]);
        iss >> *inputs[i];
    }


    istringstream iss_string(argv[6]);
    iss_string >> trace_file;

    // initialize an instance of Bimodal predictor
    Gshare gshare_predictor(i_gg, i_hh);



    // read trace file and start predicting
    vector<string> memory_trace = readTraceFile(trace_file);
    vector<string>::iterator trace_iterator;
    for (trace_iterator = memory_trace.begin(); trace_iterator != memory_trace.end(); trace_iterator++) 
    {
        gshare_predictor.prediction(*trace_iterator);
        gshare_predictor.updateGHR();
        gshare_predictor.updateCounter();
    }

    cout << "Command Line:" << endl;
    cout << "./sim_bp " << "gshare " <<  i_gg << " " <<  i_hh << " " <<  i_btb << " " << assoc_btb << " " << trace_file << endl;
    cout << endl;

    cout << "Final Gshare Table Contents: " << endl;
    for (unsigned i = 0; i < gshare_predictor.data_vec.size(); i++)
    {
        cout << "table[" << i << "]: " << gshare_predictor.data_vec[i] << endl;
    };
    cout << endl;

    cout << "Final GHR Contents: " << "0x  " << binStrToHexStr(gshare_predictor.ghr) << endl;
    cout << endl;

    cout << "Final Branch Predictor Statistics: " << endl;
    cout << "a. Number of branches: " << gshare_predictor.total_prediction << endl;
    cout << "b. Number of predictions from the branch predictor: " << gshare_predictor.total_prediction << endl;
    cout << "c. Number of mispredictions from the branch predictor: " << gshare_predictor.misprediction << endl;
    cout << "d. Number of mispredictions from the BTB: " << 0 << endl;
    cout << setprecision(2) << fixed;
    cout << "e. Misprediction Rate: " << (double)gshare_predictor.misprediction / (double)gshare_predictor.total_prediction * 100<< " percent" << endl;
}

void hybrid(int argc, char *argv[])
{
    int i_kk;
    int i_gg_gshare;
    int i_hh;
    int i_gg_bimodal;
    int i_btb;
    int assoc_btb;
    string trace_file;

    vector<int*> inputs;
    inputs.push_back(&i_kk);
    inputs.push_back(&i_gg_gshare);
    inputs.push_back(&i_hh);
    inputs.push_back(&i_gg_bimodal);
    inputs.push_back(&i_btb);
    inputs.push_back(&assoc_btb);

    for (unsigned i = 0; i < inputs.size(); i++)
    {   
        istringstream iss(argv[i+2]);
        iss >> *inputs[i];
    }


    istringstream iss_string(argv[8]);
    iss_string >> trace_file;

    // initialize an instance of Bimodal predictor
    Hybrid hybrid_predictor(i_kk, i_gg_gshare, i_hh, i_gg_bimodal);

    // read trace file and start predicting
    vector<string> memory_trace = readTraceFile(trace_file);
    vector<string>::iterator trace_iterator;
    for (trace_iterator = memory_trace.begin(); trace_iterator != memory_trace.end(); trace_iterator++) 
    {
        hybrid_predictor.prediction(*trace_iterator);
    }

    cout << "Command Line:" << endl;
    cout << "./sim_bp " << "hybrid " <<  i_kk << " " <<  i_gg_gshare << " " <<  i_hh << " " <<  i_gg_bimodal << " " << i_btb << " " << assoc_btb << " " << trace_file << endl;
    cout << endl;

    cout << "Final Bimodal Table Contents: " << endl;
    for (unsigned i = 0; i < hybrid_predictor.bimodal_predictor.data_vec.size(); i++)
    {
        cout << "table[" << i << "]: " << hybrid_predictor.bimodal_predictor.data_vec[i] << endl;
    };
    cout << endl;

    cout << "Final Gshare Table Contents: " << endl;
    for (unsigned i = 0; i < hybrid_predictor.gshare_predictor.data_vec.size(); i++)
    {
        cout << "table[" << i << "]: " << hybrid_predictor.gshare_predictor.data_vec[i] << endl;
    };
    cout << endl;

    cout << "Final GHR Contents: " << "0x  " << binStrToHexStr(hybrid_predictor.gshare_predictor.ghr) << endl;
    cout << endl;

    cout << "Final Chooser Table Contents: " << endl;
        for (unsigned i = 0; i < hybrid_predictor.chooser_table.size(); i++)
    {
        cout << "Choice table[" << i << "]: " << hybrid_predictor.chooser_table[i] << endl;
    };
    cout << endl;

    cout << "Final Branch Predictor Statistics: " << endl;
    cout << "a. Number of branches: " << hybrid_predictor.total_prediction << endl;
    cout << "b. Number of predictions from the branch predictor: " << hybrid_predictor.total_prediction << endl;
    cout << "c. Number of mispredictions from the branch predictor: " << hybrid_predictor.misprediction << endl;
    cout << "d. Number of mispredictions from the BTB: " << 0 << endl;
    cout << setprecision(2) << fixed;
    cout << "e. Misprediction Rate: " << (double)hybrid_predictor.misprediction / (double)hybrid_predictor.total_prediction * 100 << " percent" << endl;
}


void btbBimodal(int argc, char *argv[])
{
    int i_gg;
    int i_btb;
    int assoc_btb;
    string trace_file;

    vector<int*> inputs;
    inputs.push_back(&i_gg);
    inputs.push_back(&i_btb);
    inputs.push_back(&assoc_btb);

    for (unsigned i = 0; i < inputs.size(); i++)
    {   
        istringstream iss(argv[i+2]);
        iss >> *inputs[i];
    }

    istringstream iss_string(argv[5]);
    iss_string >> trace_file;

    // initialize an instance of Bimodal predictor
    Btb btb_bimodal_predictor(i_gg, i_btb, assoc_btb);

    // read trace file and start predicting
    vector<string> memory_trace = readTraceFile(trace_file);
    vector<string>::iterator trace_iterator;
    for (trace_iterator = memory_trace.begin(); trace_iterator != memory_trace.end(); trace_iterator++) 
    {
        btb_bimodal_predictor.btb_prediction(*trace_iterator);
    }

    cout << "Command Line:" << endl;
    cout << "./sim_bp " << "bimodal " <<  i_gg << " " <<  i_btb << " " << assoc_btb << " " << trace_file << endl;
    cout << endl;

    cout << "Final BTB Tag Array Contents {valid, pc}: " << endl;
    for (unsigned i = 0; i < btb_bimodal_predictor.btb_table.size(); i++)
    {
        cout << "Set     " << i << ": "; //"  {1, 0x  28f800}  {1, 0x  28d800}  {1, 0x  28fe00}  {1, 0x  28ea00}"
        for (unsigned j = 0; j < btb_bimodal_predictor.btb_assoc; j++)
        {
            cout << "{" << btb_bimodal_predictor.btb_table[i][j].valid << " ," << "0x  " << binStrToHexStr(btb_bimodal_predictor.btb_table[i][j].btb_tag) << "}  ";
        }
        cout << endl;
    }
    cout << endl;

    cout << "Final Bimodal Table Contents: " << endl;
    for (unsigned i = 0; i < btb_bimodal_predictor.bimodal_predictor->data_vec.size(); i++)
    {
        cout << "table[" << i << "]: " << btb_bimodal_predictor.bimodal_predictor->data_vec[i] << endl;
    };
    cout << endl;

    cout << "Final Branch Predictor Statistics: " << endl;
    cout << "a. Number of branches: " << btb_bimodal_predictor.total_branch << endl;
    cout << "b. Number of predictions from the branch predictor: " << btb_bimodal_predictor.bimodal_predictor->total_prediction << endl;
    cout << "c. Number of mispredictions from the branch predictor: " << btb_bimodal_predictor.bimodal_predictor->misprediction << endl;
    cout << "d. Number of mispredictions from the BTB: " << btb_bimodal_predictor.btb_misprediction << endl;
    cout << setprecision(2) << fixed;
    cout << "e. Misprediction Rate: " << ((double)btb_bimodal_predictor.btb_misprediction + (double)btb_bimodal_predictor.bimodal_predictor->misprediction) / (double)btb_bimodal_predictor.total_branch * 100 << " percent" << endl;
}

void btbGshare(int argc, char *argv[])
{
    int i_gg;
    int i_hh;
    int i_btb;
    int assoc_btb;
    string trace_file;

    vector<int*> inputs;
    inputs.push_back(&i_gg);
    inputs.push_back(&i_hh);
    inputs.push_back(&i_btb);
    inputs.push_back(&assoc_btb);

    for (unsigned i = 0; i < inputs.size(); i++)
    {   
        istringstream iss(argv[i+2]);
        iss >> *inputs[i];
    }


    istringstream iss_string(argv[6]);
    iss_string >> trace_file;

    // initialize an instance of Bimodal predictor
    Btb btb_gshare_predictor(i_gg, i_hh, i_btb, assoc_btb);

    // read trace file and start predicting
    vector<string> memory_trace = readTraceFile(trace_file);
    vector<string>::iterator trace_iterator;
    for (trace_iterator = memory_trace.begin(); trace_iterator != memory_trace.end(); trace_iterator++) 
    {
        btb_gshare_predictor.btb_prediction(*trace_iterator);
    }

    cout << "Command Line:" << endl;
    cout << "./sim_bp " << "gshare " <<  i_gg << " " <<  i_hh << " " <<  i_btb << " " << assoc_btb << " " << trace_file << endl;
    cout << endl;

    cout << "Final BTB Tag Array Contents {valid, pc}: " << endl;
    for (unsigned i = 0; i < btb_gshare_predictor.btb_table.size(); i++)
    {
        cout << "Set     " << i << ": "; //"  {1, 0x  28f800}  {1, 0x  28d800}  {1, 0x  28fe00}  {1, 0x  28ea00}"
        for (unsigned j = 0; j < btb_gshare_predictor.btb_assoc; j++)
        {
            cout << "{" << btb_gshare_predictor.btb_table[i][j].valid << " ," << "0x  " << binStrToHexStr(btb_gshare_predictor.btb_table[i][j].btb_tag) << "}  ";
        }
        cout << endl;
    }
    cout << endl;

    cout << "Final Gshare Table Contents: " << endl;
    for (unsigned i = 0; i < btb_gshare_predictor.gshare_predictor->data_vec.size(); i++)
    {
        cout << "table[" << i << "]: " << btb_gshare_predictor.gshare_predictor->data_vec[i] << endl;
    };
    cout << endl;

    cout << "Final GHR Contents: " << "0x  " << binStrToHexStr(btb_gshare_predictor.gshare_predictor->ghr) << endl;
    cout << endl;

    cout << "Final Branch Predictor Statistics: " << endl;
    cout << "a. Number of branches: " << btb_gshare_predictor.total_branch << endl;
    cout << "b. Number of predictions from the branch predictor: " << btb_gshare_predictor.gshare_predictor->total_prediction << endl;
    cout << "c. Number of mispredictions from the branch predictor: " << btb_gshare_predictor.gshare_predictor->misprediction << endl;
    cout << "d. Number of mispredictions from the BTB: " << btb_gshare_predictor.btb_misprediction << endl;
    cout << setprecision(2) << fixed;
    cout << "e. Misprediction Rate: " << ((double)btb_gshare_predictor.btb_misprediction + (double)btb_gshare_predictor.gshare_predictor->misprediction) / (double)btb_gshare_predictor.total_branch * 100 << " percent" << endl;
}


void btbHybrid(int argc, char *argv[])
{
    int i_kk;
    int i_gg_gshare;
    int i_hh;
    int i_gg_bimodal;
    int i_btb;
    int assoc_btb;
    string trace_file;

    vector<int*> inputs;
    inputs.push_back(&i_kk);
    inputs.push_back(&i_gg_gshare);
    inputs.push_back(&i_hh);
    inputs.push_back(&i_gg_bimodal);
    inputs.push_back(&i_btb);
    inputs.push_back(&assoc_btb);

    for (unsigned i = 0; i < inputs.size(); i++)
    {   
        istringstream iss(argv[i+2]);
        iss >> *inputs[i];
    }


    istringstream iss_string(argv[8]);
    iss_string >> trace_file;

    // initialize an instance of Bimodal predictor
    Btb btb_hybrid_predictor(i_kk, i_gg_gshare, i_hh, i_gg_bimodal, i_btb, assoc_btb);

    // read trace file and start predicting
    vector<string> memory_trace = readTraceFile(trace_file);
    vector<string>::iterator trace_iterator;
    for (trace_iterator = memory_trace.begin(); trace_iterator != memory_trace.end(); trace_iterator++) 
    {
        btb_hybrid_predictor.btb_prediction(*trace_iterator);
    }

    cout << "Command Line:" << endl;
    cout << "./sim_bp " << "hybrid " <<  i_kk << " " <<  i_gg_gshare << " " <<  i_hh << " " <<  i_gg_bimodal << " " << i_btb << " " << assoc_btb << " " << trace_file << endl;
    cout << endl;

    cout << "Final BTB Tag Array Contents {valid, pc}: " << endl;
    for (unsigned i = 0; i < btb_hybrid_predictor.btb_table.size(); i++)
    {
        cout << "Set     " << i << ": "; //"  {1, 0x  28f800}  {1, 0x  28d800}  {1, 0x  28fe00}  {1, 0x  28ea00}"
        for (unsigned j = 0; j < btb_hybrid_predictor.btb_assoc; j++)
        {
            cout << "{" << btb_hybrid_predictor.btb_table[i][j].valid << " ," << "0x  " << binStrToHexStr(btb_hybrid_predictor.btb_table[i][j].btb_tag) << "}  ";
        }
        cout << endl;
    }
    cout << endl;

    cout << "Final Bimodal Table Contents: " << endl;
    for (unsigned i = 0; i < btb_hybrid_predictor.hybrid_predictor->bimodal_predictor.data_vec.size(); i++)
    {
        cout << "table[" << i << "]: " << btb_hybrid_predictor.hybrid_predictor->bimodal_predictor.data_vec[i] << endl;
    };
    cout << endl;

    cout << "Final Gshare Table Contents: " << endl;
    for (unsigned i = 0; i < btb_hybrid_predictor.hybrid_predictor->gshare_predictor.data_vec.size(); i++)
    {
        cout << "table[" << i << "]: " << btb_hybrid_predictor.hybrid_predictor->gshare_predictor.data_vec[i] << endl;
    };
    cout << endl;

    cout << "Final GHR Contents: " << "0x  " << binStrToHexStr(btb_hybrid_predictor.hybrid_predictor->gshare_predictor.ghr) << endl;
    cout << endl;

    cout << "Final Chooser Table Contents: " << endl;
        for (unsigned i = 0; i < btb_hybrid_predictor.hybrid_predictor->chooser_table.size(); i++)
    {
        cout << "Choice table[" << i << "]: " << btb_hybrid_predictor.hybrid_predictor->chooser_table[i] << endl;
    };
    cout << endl;

    cout << "Final Branch Predictor Statistics: " << endl;
    cout << "a. Number of branches: " << btb_hybrid_predictor.total_branch << endl;
    cout << "b. Number of predictions from the branch predictor: " << btb_hybrid_predictor.hybrid_predictor->total_prediction << endl;
    cout << "c. Number of mispredictions from the branch predictor: " << btb_hybrid_predictor.hybrid_predictor->misprediction << endl;
    cout << "d. Number of mispredictions from the BTB: " << btb_hybrid_predictor.btb_misprediction << endl;
    cout << setprecision(2) << fixed;
    cout << "e. Misprediction Rate: " << ((double)btb_hybrid_predictor.hybrid_predictor->misprediction + (double)btb_hybrid_predictor.btb_misprediction) / (double)btb_hybrid_predictor.total_branch * 100 << " percent" << endl;
}


/*
Main function of the predictor:
    read different numbers of argument from the command line, then according to the arguemnt, invoke the corresponding predictor.
*/

int main(int argc, char *argv[])  
{   
    string mode;

    istringstream iss_string(argv[1]);
    iss_string >> mode;

    if (mode == "bimodal")
    {   
        if (*argv[3] == '0' || *argv[4] == '0')
        {
            bimodal(argc, argv);
        }
        else
        {
            btbBimodal(argc, argv);
        }
    }
    
    else if (mode == "gshare")
    {   
        if (*argv[4] == '0' || *argv[5] == '0')
        {
            gshare(argc, argv);
        }
        else
        {
            btbGshare(argc, argv);
        }
    }

    else if (mode == "hybrid")
    {
        if (*argv[6] == '0' || *argv[7] == '0')
        {
            hybrid(argc, argv);
        }
        else
        {
            btbHybrid(argc, argv);
        }
    }

    return 0;
}





















