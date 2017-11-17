#ifndef btb_h
#define btb_h

#include "common.h"
#include "util.h"
#include "bimodal.h"
#include "gshare.h"
#include "hybrid.h"

// To use pow
#include <cmath>

/*
Data structure for BTB cell
- @btb_tag: string, to hold tag
- @valid: unsigned, to hold valid bit. valid == 1 means the cell is already used, thus unavailable; valid == 0 means the cell the empty, thus available. valid is initialized to 1 at the beginning of each simulation
- @lru_data: unsigned, to hold LRU aging information. The bigger the value of lru_data, the more recently used it is
*/
struct btbCell
{
    string btb_tag;
    unsigned valid;
    unsigned lru_data;
};

typedef vector<btbCell> btbRow;
typedef vector<btbRow> btbTable;


// Helper function used to find the victim cell using LRU replacement policy
struct MinMax
{
    int min;
    int max;
};

MinMax findMinMaxLru(btbRow &row_data) 
{   
    MinMax result;
    result.min = row_data[0].lru_data;
    result.max = 0;

    for (unsigned i = 0; i < row_data.size(); i++) 
    {
        if (result.min > row_data[i].lru_data)
        {
            result.min = row_data[i].lru_data;
        }

        if (result.max < row_data[i].lru_data)
        {
            result.max = row_data[i].lru_data;
        }
    }

    return result;
}

/*
Main class for btb.

Three parameters will be provided to the constructor: 
@i_bb: number of bits used in PC to index 
@assoc_btb: associativity of each row of the BTB

Member variables:
- total number of branch: @total_branch
- mispredictions of btb: @btb_misprediction
- data table: @btb_table
- number of bits used in PC to index btb table: @i_btb
- predictor to use: @branch_predictor
*/
class Btb
{
public:
    int total_branch;
    int btb_misprediction;
    int i_btb;
    int btb_assoc;
    btbTable btb_table;

    Bimodal *bimodal_predictor;
    Gshare *gshare_predictor;
    Hybrid *hybrid_predictor;

    /* 
    There are three contructors, each contructor corresponds to a specific branch predictor. Since each predictor requires a different numbre of parameters, each constructor can be distinguished by the number of parameters provided

    Common to all constructors, initialize the values of member variables:
    - total_branch: 0
    - btb_misprediction: 0
    - btb_table: initialize size according to i_btb and assoc, initialize value to be ""
    */

    // Contructor for btb using bimodal
    Btb (unsigned i_gg, unsigned i_bb, unsigned assoc): total_branch(0), btb_misprediction(0), i_btb(i_bb), btb_assoc(assoc)
    {   
        // initialize bimodal_predictor
        bimodal_predictor = new Bimodal(i_gg);
        gshare_predictor = NULL;
        hybrid_predictor = NULL;

        // initialize btb_table
        btbCell btb_cell;
        btb_cell.valid = 0;
        btb_cell.lru_data = 0;
        // btb_cell.btb_tag is empty by default
        btb_cell.btb_tag = "";

        btbRow btb_row;
        for (unsigned i = 0; i < assoc; i++)
        {
            btb_row.push_back(btb_cell);
        }

        //unsigned row_num = util::sniePow(2, i_bb);
        unsigned row_num = pow(2, i_bb);
        for (unsigned i = 0; i < row_num; i++)
        {
            btb_table.push_back(btb_row);
        }
    }

    // Contructor for btb using gshare
    Btb (unsigned i_gg, unsigned i_h, unsigned i_bb, unsigned assoc): total_branch(0), btb_misprediction(0), i_btb(i_bb), btb_assoc(assoc)
    {   
        // initialize bimodal_predictor
        gshare_predictor = new Gshare(i_gg, i_h);
        bimodal_predictor = NULL;
        hybrid_predictor = NULL;

        // initialize btb_table
        btbCell btb_cell;
        btb_cell.valid = 0;
        btb_cell.lru_data = 0;
        // btb_cell.btb_tag is empty by default

        btbRow btb_row;
        for (unsigned i = 0; i < assoc; i++)
        {
            btb_row.push_back(btb_cell);
        }

        unsigned row_num = util::sniePow(2, i_bb);
        for (unsigned i = 0; i < row_num; i++)
        {
            btb_table.push_back(btb_row);
        }
    }


    // Contructor for btb using hybrid
    Btb (unsigned i_kk, unsigned i_gg_gshare, unsigned i_hh, unsigned i_gg_bimodal, unsigned i_bb, unsigned assoc): total_branch(0), btb_misprediction(0), i_btb(i_bb), btb_assoc(assoc)
    {   
        // initialize bimodal_predictor

        hybrid_predictor = new Hybrid(i_kk, i_gg_gshare, i_hh, i_gg_bimodal);
        bimodal_predictor = NULL;
        gshare_predictor = NULL;

        // initialize btb_table
        btbCell btb_cell;
        btb_cell.valid = 0;
        btb_cell.lru_data = 0;
        // btb_cell.btb_tag is empty by default

        btbRow btb_row;
        for (unsigned i = 0; i < assoc; i++)
        {
            btb_row.push_back(btb_cell);
        }

        unsigned row_num = util::sniePow(2, i_bb);
        for (unsigned i = 0; i < row_num; i++)
        {
            btb_table.push_back(btb_row);
        }

    }

    void btb_prediction(string &instruction);
};


/*
Input:
@instruction: binary string from tracefile
the first bits are the branch's actual direction; seperated by a space, the rest of the string the binary string of the instruction itself

Steps for btb prediction:
1. Read an instruction. Ignore the lowest 2 bits of the PC, use the next higher i_btb bits to index btb_table. Use the rest of the higher bits as the tag
2. Use the tag to check the index row. Search through the row to see if there is a tag match
3. If there is a match, use the branch_predictor to predict. Meanwhile, update lru_data
4. If there is no match, do the following:
    - Predict the branch as non-taken. No prediction happens.
    - Install the new PC: check valid cell first, if there is a valid cell, install it there; if not, use LRU policy to find a victim PC. No writeback. Remember to update lru_data at the same time.
    - Install the tag to the found cell
    - Compare the implicit prediction(non-taken) to the actual direction. If the actual direction is taken, btb_misprediction++
*/

void Btb::btb_prediction(string &instruction)
{   
    // Step 1
    total_branch++;

    // Seperate the acutal direction from the PC address
    string actual_direction = instruction.substr(0, 1);
    string bin_str = instruction.substr(2, instruction.size() - 2);

    // Read the index and the tag from the PC address
    string btb_index_str = bin_str.substr(bin_str.size() - 2 - i_btb, i_btb);
    //string tag_str = bin_str.substr(0, bin_str.size() - 2 - i_btb);
    // use the whole PC as tag
    string tag_str = bin_str;

    int current_index_dec = util::binToDec(btb_index_str.c_str());

    // Step 2
    bool hit = false;
    unsigned hit_assoc;
    for (unsigned i = 0; i < btb_assoc; i++)
    {
        if (btb_table[current_index_dec][i].btb_tag == tag_str)
        {
            hit = true;
            hit_assoc = i;

            // Always remeber to break once there is a hit
            break;
        }
    }

    // Find the current min and max lru value in the row, to be used to update lru_data
    MinMax min_max_lru_data = findMinMaxLru(btb_table[current_index_dec]);


    // Step 3: Found a matched tag    
    if (hit == true)
    {
        // use the predictor to predict branch
        if (bimodal_predictor == NULL)
        {
            if (gshare_predictor == NULL)
            {
                hybrid_predictor->prediction(instruction);
            }
            else
            {
                gshare_predictor->prediction(instruction);
                gshare_predictor->updateGHR();
                gshare_predictor->updateCounter();
            }
        }
        else
        {
            bimodal_predictor->prediction(instruction);
            bimodal_predictor->updateCounter();
        }

        // update lru data
        btb_table[current_index_dec][hit_assoc].lru_data = min_max_lru_data.max + 1;
    }


    // Step 4: No matched tag found
    else
    {   
        /* 
        Check first if there is a valid cell. 
        If there is, install the tag in the valid cell;
        Also, update lru_data!
        */
        bool valid_cell = false;
        for (unsigned i = 0; i < btb_assoc; i++)
        {   
            if (btb_table[current_index_dec][i].valid == 0)
            {
                btb_table[current_index_dec][i].btb_tag = tag_str;
                btb_table[current_index_dec][i].valid = 1;
                btb_table[current_index_dec][i].lru_data = min_max_lru_data.max + 1;
                valid_cell = true;

                // Always remeber to break once there is a hit
                break;
            }
        }

        /* 
        If there is no valid cell:
        - Use LRU policy to find a victim cell
        - Install the new PC tag into the cell
        - Update lru_data of the cell
        */
        if (valid_cell == false)
        {
            for (unsigned i = 0; i < btb_assoc; i++)
            {
                if (btb_table[current_index_dec][i].lru_data == min_max_lru_data.min)
                {
                    btb_table[current_index_dec][i].btb_tag = tag_str;

                    btb_table[current_index_dec][i].lru_data = min_max_lru_data.max + 1;

                    // btb_table[current_index_dec][i].valid = 1;

                    // Always remeber to break once there is a hit
                    break;
                }
            }
        }

        // Compare the implicit prediction(non-taken) to the actual direction. If the actual direction is taken, btb_misprediction++
        if (actual_direction == "t")
        {
            btb_misprediction++;
        }
    }
}


#endif































