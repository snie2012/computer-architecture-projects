#ifndef cache_h
#define cache_h

#include "common.h"

struct MinMax
{
    int min;
    int max;
};

struct TagCell 
{
    string tag_data;
    int meta_data;
    unsigned dirty_flag;
    unsigned available;
};

typedef vector<TagCell> TagRow;
typedef vector<TagRow> TagMatrix;

MinMax findMaxMinTag(TagRow &row_data);
bool checkAvailability(TagRow &row_data);


class Cache 
{
private:
    int offset_width;
    int ways;
    int sets;
    int index_width;
    int repl;
    int count;
    vector<int> lfu_count_set;

    TagMatrix tag_matrix;

    void LRU(string& access_type, unsigned hit, int hit_id, int set_id, string& tag_data);
    void FIFO(string& access_type, unsigned hit, int hit_id, int set_id, string& tag_data);
    void LFU(string& access_type, unsigned hit, int hit_id, int set_id, string& tag_data);
    void writeBack(unsigned replace, string& access_type, int set_id, int index);
    

public:
    unsigned read_count;
    unsigned read_miss;
    unsigned write_count;
    unsigned write_miss;
    unsigned write_back;

    Cache(int block_size, int cache_size, int set_assoc, int repl_pc);
    void fetchTag(string& instruction);
};

#endif





















