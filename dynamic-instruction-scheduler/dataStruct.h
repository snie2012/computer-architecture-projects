#ifndef dataStruct_h
#define dataStruct_h
/*
Build data structures for:
    - Reanme map table
    - Issue queue
    - Reorder buffer
*/

struct cycleInfo
{
    int start_cycle;
    int cycle_num;
};

struct instructionEntry
{
    int pc;
    int type;
    int des;
    int src1;
    int src2;

    int timer;

    int rob_des;
    int rob_src1;
    int rob_src2;

    int src1_ready;
    int src2_ready;

    cycleInfo FE, DE, RN, RR, DI, IS, EX, WB, RT;
};


struct renameMapTableEntry
{
    int valid;
    int rob_entry;
};


struct issueQueueEntry
{
    int valid;
    int age;
    instructionEntry inst_entry;
};


struct reorderBufferEntry
{
    int des_reg;

    // 0: not ready; 1: ready; -1: not used
    int ready;

    instructionEntry inst_entry;
};


#endif




