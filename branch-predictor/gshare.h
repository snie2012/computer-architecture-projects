#ifndef gshare_h
#define gshare_h

#include "common.h"

// Used for power calculation
#include "util.h"

class Gshare
/* 
The class structure is similar to Bimodal class, but there are two differences:
    - there are two additional members @i_h and @ghr used for gshare indexing

The main different between Bimodal and Gshare will be the way they make predictions, which will be reflected in the @prediction function
*/
{
public:
    int i_g; // Store pc index length
    int i_h; // Store ghr length
    int total_prediction; // Store the total number of prediction
    int misprediction; // Store the number of misprediction
    string actual_direction; // Store the result of each actual direction whether a branch is taken
    string predicted_direction; // Store the result of each prediction
    string ghr; // Store ghr used for indexing
    vector<unsigned> data_vec; // Store the actual data table for prediction
    int current_entry; // The current entry the predictor is reading and writing

    Gshare(unsigned i_gg, unsigned i_hh): total_prediction(0), misprediction(0), actual_direction(""), predicted_direction(""), current_entry(-1), i_g(i_gg), i_h(i_hh)
    {
        unsigned entry_num = util::sniePow(2, i_gg);

        for (unsigned i = 0; i < entry_num; i++)
        {
            data_vec.push_back(2);
        }

        // ghr is initialized to all zeros at the beginning of each simulation
        for (unsigned i = 0; i < i_h; i++)
        {
            ghr += '0';
        }
    };

    void prediction(string &instruction);
    void updateGHR();
    void updateCounter();
};



void Gshare::prediction(string &instruction)
/*
The process of gshare prediction:
1. Take an instruction. Store the binary part and label part (actual taken or not) seperately
    - increment total_prediction by 1 each time an instruction is read
2. Do XOR operation with the current h-bit ghr and the uppermost h bits of the i-bits supplied from PC. Combine the result of the operation and the rest i-h bits to form the final binary string. Then convert binary string to decimal.
3. Use the calculated decimal to index the gshare predictor. Read the indexed value in the predictor
4. If the value in the predictor is larger or equal to 2, then predict take; otherwise, predict not-take
5. Compare the predicted result with the actual result. If inconsistent, increment mis_prediction by 1
6. Read the actual direction, if taken, increase the counter in the current entry by 1; else decrease 1. Notice: the counter value cannot be smaller than 0 or larger than 3
7. While reading the actual direction in step 6, also update ghr: shift ghr right by 1 position and place the branch's actual outcome (1 for actually taken, 0 for actually non-taken) into the leftmost bit position

@instruction: string. The first position of the string is the actual direction: 'n' or 't'. Seperated by a space, the rest of the string is a string of binary numbers
*/

{   
    total_prediction++;

    actual_direction = instruction.substr(0, 1);

    // Calculate gshare index
    string bin_str = instruction.substr(2, instruction.size() - 2);
    
    // 1. XOR operation
    string upper_h_str = bin_str.substr(bin_str.size() - 2 - i_g, i_h);
    string xor_res_str = util::xorBinStr(upper_h_str, ghr);

    // 2. Combine the xor result with the rest i-h bits
    string index_str = xor_res_str + bin_str.substr(bin_str.size()- 2 - i_g + i_h, i_g - i_h);

    //cout << "xor_res_str: " << xor_res_str << endl;
    //cout << "index_str: " << index_str << endl;

    //convert binary string to decimal integer
    current_entry = util::binToDec(index_str.c_str());
    //cout << "current_entry: " << current_entry << endl;

    // read entry value and predict direction
    if (data_vec[current_entry] >= 2)
    {
        predicted_direction = "t";
    }
    else
    {
        predicted_direction = "n";   
    }

    // compare actual direction with predicted direction
    if (predicted_direction != actual_direction)
    {
        misprediction++;
    }
}

/*
Seperate the update process from the prediction process since these two process has to be treated differently in hybrid predictor.
The ghr update process and conter update process have also to be seperated.
*/
void Gshare::updateGHR()
{
    // shift ghr to right by 1 bit
    for (unsigned i = ghr.length() - 1; i >= 1; i--)
    {
        ghr[i] = ghr[i-1];
    }

    // use the actual direction to update the leftmost bit of ghr
    if (actual_direction == "t")
    {
        ghr[0] = '1';
    }
    else
    {
        ghr[0] = '0';
    }
}


void Gshare::updateCounter()
{
     // use the actual direction to update the counter at the current entry
    if (actual_direction == "t")
    {
        // always check if the value is in the range of [0, 3] first
        data_vec[current_entry] == 3 ? NULL : data_vec[current_entry]++;
    }
    else
    {
        data_vec[current_entry] == 0 ? NULL : data_vec[current_entry]--;
    }
}



#endif



























