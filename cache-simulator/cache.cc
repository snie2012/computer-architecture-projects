#include "cache.h"

#include "util.h"
#include <cmath>

MinMax findMaxMinTag(TagRow &row_data) 
{
    MinMax max_min;

    max_min.max = -1;
    max_min.min = row_data[0].meta_data;

    for (unsigned i = 0; i < row_data.size(); i++) 
    {
        if (max_min.max < row_data[i].meta_data)
            max_min.max = row_data[i].meta_data;
        if (max_min.min > row_data[i].meta_data)
            max_min.min = row_data[i].meta_data;
    }

    return max_min;
}

bool checkAvailability(TagRow &row_data) {
    for (unsigned i = 0; i < row_data.size(); i++) {
        if (row_data[i].available == 1)
            return true;
    }
    return false;
}


Cache::Cache (int block_size, int cache_size, int set_assoc, int repl_pc) 
{   
    repl = repl_pc;
    ways = set_assoc;
    sets = cache_size / (block_size * ways);
    index_width = log2(sets);
    offset_width = log2(block_size);

    read_count = 0;
    write_count = 0;
    read_miss = 0;
    write_miss = 0;
    write_back = 0;

    count = 0;

    TagCell tag_cell;
    tag_cell.tag_data = "-1";
    tag_cell.meta_data = 0;
    tag_cell.dirty_flag = 0;
    tag_cell.available = 1;

    TagRow tag_row;

    for (unsigned i = 0; i < ways; i++) 
    {
        tag_row.push_back(tag_cell);
    }

    for (unsigned i = 0; i < sets; i++) 
    {
        tag_matrix.push_back(tag_row);
        //if (repl == 2)
        lfu_count_set.push_back(0);
    }

    // cout << "tag matrix: " << tag_matrix[0][0].tag_data << endl;
    // cout << "block size: " << block_size << endl;
    // cout << "sets: " << sets << endl;
    // cout << "ways: " << ways << endl;
    // cout << "index width: " << index_width << endl;
}


void Cache::fetchTag (string &instruction) 
{

    int tag_width = instruction.length() - 2 - index_width - offset_width;

    // convert binary string to int
    //int set_id = stoi(instruction.substr(2 + tag_width, index_width), 0, 2); // only supported in c++11
    const char *bin = instruction.substr(2 + tag_width, index_width).c_str();
    int set_id = util::binToDec(bin);

    string tag = instruction.substr(2, tag_width);
    string access_type = instruction.substr(0, 1);

    access_type == "r" ? read_count++ : write_count++;

    unsigned hit = 0;
    unsigned hit_id;

    for (unsigned i = 0; i < ways; i++) 
    {
        if (tag == tag_matrix[set_id][i].tag_data) 
        {
            hit = 1;
            hit_id = i;
            break; // break as long as a tag match is found
        }
    }

    if (hit == 0) 
    {   
        access_type == "r" ? read_miss++ : write_miss++;
        hit_id = -1;
    }

    switch (repl) {
        case 0:
            LRU(access_type, hit, hit_id, set_id, tag);
            break;;
        case 1:
            FIFO(access_type, hit, hit_id, set_id, tag);
            break;
        case 2:
            LFU(access_type, hit, hit_id, set_id, tag);
            break;
    }

    

    /*
    cout << "-----------------------------------------------------------" << endl;
    cout << "#" << count << " : " << instruction << endl;
    cout << "============    Simulation results   ============" << endl;
    cout << "a. number of L1 reads:             " << read_count << endl;
    cout << "b. number of L1 read misses:       " << read_miss << endl;
    cout << "c. number of L1 writes:            " << write_count << endl;
    cout << "d. number of L1 write misses:      " << write_miss << endl;
    cout << "e. L1 miss rate:                   " << (double)(read_miss + write_miss)/(double)(read_count + write_count) << endl;
    cout << "f. number of L1->L2 writebacks:    " << write_back << endl;
    cout << "g. number of L2 reads:             " << 0 << endl;
    cout << "h. number of L2 read misses:       " << 0 << endl;
    cout << "i. number of L2 writes:            " << 0 << endl;
    cout << "j. number of L2 write misses:      " << 0 << endl;
    cout << "k. L2 miss rate:                   " << (double)0.0000 << endl;
    cout << "l. number of L2->Mem writebacks:   " << 0 << endl;
    cout << "m. total memory traffic:           " << read_miss + write_miss + write_back<< endl;
    */
}


void Cache::LFU(string &access_type, unsigned hit, int hit_id, int set_id, string &tag_data) 
{   
    unsigned index;

    if (hit == 0) 
    {   
        MinMax max_min = findMaxMinTag(tag_matrix[set_id]);
        if (checkAvailability(tag_matrix[set_id])) 
        {
            for (unsigned i = 0; i < ways; i++) {
                if (tag_matrix[set_id][i].available == 1) {
                    lfu_count_set[set_id] = max_min.min;
                    tag_matrix[set_id][i].meta_data = lfu_count_set[set_id] + 1;
                    tag_matrix[set_id][i].tag_data = tag_data;

                    tag_matrix[set_id][i].available = 0;
                    index = i;
                    break;
                }
            }
        } 
        else
        { 
            for (unsigned i = 0; i < ways; i++) {
                if (tag_matrix[set_id][i].meta_data == max_min.min) {
                    lfu_count_set[set_id] = max_min.min;
                    tag_matrix[set_id][i].meta_data = lfu_count_set[set_id] + 1;
                    tag_matrix[set_id][i].tag_data = tag_data;

                    index = i;
                    break; // break as long as a min value is found (replace the leftmost cell in a row.
                }
            }
        }
    } 
    else 
    {
        tag_matrix[set_id][hit_id].meta_data++;
        index = hit_id;
    }   

    writeBack(hit, access_type, set_id, index);
}


void Cache::LRU(string &access_type, unsigned hit, int hit_id, int set_id, string &tag_data) 
{
    MinMax max_min = findMaxMinTag(tag_matrix[set_id]);
    unsigned index;

    if (hit == 0) 
    {   
        if (checkAvailability(tag_matrix[set_id])) 
        {
            for (unsigned i = 0; i < ways; i++) {
                if (tag_matrix[set_id][i].available == 1) {
                    tag_matrix[set_id][i].meta_data = max_min.max + 1;
                    tag_matrix[set_id][i].tag_data = tag_data;
                    index = i;

                    tag_matrix[set_id][i].available = 0;
                    break;
                }
            }
        }
        else 
        {
            for (unsigned i = 0; i < ways; i++) 
            {
                if (tag_matrix[set_id][i].meta_data == max_min.min) 
                {                    
                    tag_matrix[set_id][i].meta_data = max_min.max + 1;
                    tag_matrix[set_id][i].tag_data = tag_data;
                    index = i;
                    break; // break as long as a min value is found (replace the leftmost cell in a row)
                }
            }
        }
    } 
    else 
    {        
        tag_matrix[set_id][hit_id].meta_data = max_min.max + 1;
        index = hit_id;
    }

    writeBack(hit, access_type, set_id, index);
}


void Cache::FIFO(string &access_type, unsigned hit, int hit_id, int set_id, string &tag_data) 
{
    unsigned index; 

    if (hit == 0) 
    {
        MinMax max_min = findMaxMinTag(tag_matrix[set_id]);

        if (checkAvailability(tag_matrix[set_id])) 
        {
            for (unsigned i = 0; i < ways; i++) {
                if (tag_matrix[set_id][i].available == 1) {
                    tag_matrix[set_id][i].meta_data = 0;
                    tag_matrix[set_id][i].tag_data = tag_data;
                    index = i;

                    tag_matrix[set_id][i].available = 0;
                    break;
                }
            }
        }
        else 
        {
            for (unsigned i = 0; i < ways; i++) 
            {
                if (tag_matrix[set_id][i].meta_data == max_min.max) 
                {
                    tag_matrix[set_id][i].meta_data = 0;
                    tag_matrix[set_id][i].tag_data = tag_data;
                    index = i;
                    break; // break as long as a min value is found (replace the leftmost cell in a row)
                }
            }
        }

        for (unsigned i = 0; i < ways; i++) 
        {
            if (i != index) 
                tag_matrix[set_id][i].meta_data++;
        }
    } 

    else 
    {
        for (unsigned i = 0; i < ways; i++) 
        {
            tag_matrix[set_id][i].meta_data++;
        }

        index = hit_id;
    }

    writeBack(hit, access_type, set_id, index);
}


void Cache::writeBack(unsigned hit, string &access_type, int set_id, int index) 
{   
    if (hit == 1)
    {
        access_type == "w" ? tag_matrix[set_id][index].dirty_flag = 1 : NULL;
    }
    else
    {   
        tag_matrix[set_id][index].dirty_flag == 1 ? write_back++ : NULL;
        access_type == "w" ? tag_matrix[set_id][index].dirty_flag = 1 : tag_matrix[set_id][index].dirty_flag = 0;
    }
}
















