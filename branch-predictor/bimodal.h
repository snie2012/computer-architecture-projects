#ifndef bimodal_h
#define bimodal_h

#include "common.h"

// Used for power calculation
#include "util.h"

/* 
Define the class for bimodal predictor, member to include:
    - total_prediction: total predictions the predictor made
    - misprediction: total mispredictions thee predictor made
    - data_vec: use a vector to simulator the bimodal predictor. Each entry of the vector is an unsigned number, which can only be one of the four values (0, 1, 2, 3)
*/
class Bimodal
{
public:
    int i_pc;
    int total_prediction;
    int misprediction;
    string actual_direction;
    string predicted_direction;
    vector<unsigned> data_vec;
    int current_entry;

    /* 
    Initializer of the Bimodal class:
        - initialize the value of total_prediction and misprediction to be 0
        - initialize i_pc to be the index_width. To be use in the prediction function
        - index_width: the number of index bits given as a command line argument, determining the number of entries of the predictor. If the index_width is i, then the number of entries is 2^i

    Once the number of entries is calculated, use it to initialize data_vec. Initially, all the value of data_vec's entries is set to 2
    */
    Bimodal(unsigned index_width): total_prediction(0), misprediction(0), actual_direction(""), predicted_direction(""), i_pc(index_width), current_entry(-1)
    {
        unsigned entry_num = util::sniePow(2, index_width);
        for (unsigned i = 0; i < entry_num; i++)
        {
            data_vec.push_back(2);
        }
    };

    void prediction(string &instruction);
    void updateCounter();
};



/*
The process of bimodal prediction:
1. Take an instruction. Store the binary part and label part (actual taken or not) seperately
    - increment total_prediction by 1 each time reading an instruction
2. Use i_pc argument to determine how many bits to be used as index. Convert these bits to decimal
3. Use the calculated decimal to index the bimodal predictor. Read the indexed value in the predictor
4. If the value in the predictor is larger or equal to 2, then predict take; otherwise, predict not-take
5. Compare the predicted result with the actual result. If inconsistent, increment mis_prediction by 1
6. Read the actual direction, if taken, increase the counter in the current entry by 1; else decrease 1. Notice: the counter value cannot be smaller than 0 or larger than 3

@instruction: string. The first position of the string is the actual direction: 'n' or 't'. Seperated by a space, the rest of the string the binary address
*/
void Bimodal::prediction(string &instruction)
{   
    total_prediction++;

    actual_direction = instruction.substr(0, 1);

    // index the instruction and convert binary string to decimal integer
    string bin_str = instruction.substr(2, instruction.size() - 2);
    
    const char *bin_ptr = bin_str.substr(bin_str.size()-2-i_pc, i_pc).c_str();
    current_entry = util::binToDec(bin_ptr);
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
*/

void Bimodal::updateCounter()
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