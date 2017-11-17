#ifndef hybrid_h
#define hybrid_h

#include "common.h"
#include "util.h"
#include "bimodal.h"
#include "gshare.h"

class Hybrid
/* 
The class structure is similar to Bimodal class, but there are two differences:
    - there are two additional members @i_h and @ghr used for gshare indexing

The main different between Bimodal and Gshare will be the way they make predictions, which will be reflected in the @prediction function
*/
{
public:
    int i_k; // Number of bits of chooser table size
    
    int total_prediction;
    int misprediction;
    vector<unsigned> chooser_table; // Store the chooser table for hybrid predictor

    Bimodal bimodal_predictor;
    Gshare gshare_predictor;

    Hybrid(unsigned i_kk, unsigned i_gg_gshare, unsigned i_hh, unsigned i_gg_bimodal): total_prediction(0), misprediction(0),i_k(i_kk), bimodal_predictor(i_gg_bimodal), gshare_predictor(i_gg_gshare, i_hh) // Initialize the bimodal and gshare predictor
    {   
        // All counters of the chooser table is initialized to 1 at the beginning of each simulation
        unsigned chooser_table_num = util::sniePow(2, i_k);
        for (unsigned i = 0; i < chooser_table_num; i++)
        {
            chooser_table.push_back(1);
        }
    };

    void prediction(string &instruction);
};



void Hybrid::prediction(string &instruction)
/*
The process of gshare prediction:
1. Obtain two predictions from bimodal predictor and gshare predictor
2. Use bits k+1 to 2 of the branch PC to index the chooser table
3. Read the counter value of the index chooser table entry, if counter >= 2, use gshare; otherwise use bimodal
4. Update only the chosen branch predictor based on the branch's actual outcome
5. Update gshare's ghr regardless of the predictor chosen
6. Update the branch's chooser counter:
    if prediction from both predictor are correct or incorrect, then leave the chooser counter unchanged;
    if gshare is correct and bimodal is incorrect, then increment the counter (saturate at 3);
    if bimodal is correct and gshare is incorrect, then decrement the counter (saturate at 0);

@instruction: string. The first position of the string is the actual direction: 'n' or 't'. Seperated by a space, the rest of the string is a string of binary numbers
*/

{   
    total_prediction++;
    string actual_direction = instruction.substr(0, 1);

    // Step 1
    bimodal_predictor.prediction(instruction);
    gshare_predictor.prediction(instruction);

    // Step 2
    string bin_str = instruction.substr(2, instruction.size() - 2);
    string k_bin_str = bin_str.substr(bin_str.size() - 2 - i_k, i_k);
    int chooser_id = util::binToDec(k_bin_str.c_str());

    // Step 3 and 4
    string predicted_direction;
    if (chooser_table[chooser_id] >= 2)
    {
        predicted_direction = gshare_predictor.predicted_direction;
        gshare_predictor.updateCounter();
    }
    else
    {
        predicted_direction = bimodal_predictor.predicted_direction;
        bimodal_predictor.updateCounter();
    }

    // Compare actual direction with predicted direction, update misprediction
    if (predicted_direction != actual_direction)
    {
        misprediction++;
    }

    // Step 5
    gshare_predictor.updateGHR();

    // Step 6
    if (gshare_predictor.predicted_direction == actual_direction && bimodal_predictor.predicted_direction != actual_direction)
    {
        chooser_table[chooser_id] == 3 ? NULL : chooser_table[chooser_id]++;
    }

    if (gshare_predictor.predicted_direction != actual_direction && bimodal_predictor.predicted_direction == actual_direction)
    {
        chooser_table[chooser_id] == 0 ? NULL : chooser_table[chooser_id]--;
    }
}


#endif