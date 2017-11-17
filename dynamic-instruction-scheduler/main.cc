#include "common.h"
#include "readFile.h"
#include "dataStruct.h"

#include <iomanip>
#include <sstream>

#include <algorithm>
#include <cmath>

/*
 Global variables. Used by many funcitons
 */
int arch_reg_num;
int iq_size;
int rob_size;
int width;
vector<int> last_wakeup_list;

vector<renameMapTableEntry> rnm_table;
vector<issueQueueEntry> iq_table;

vector<reorderBufferEntry> rob_table;
int head;
int tail;

vector<instructionEntry> decode_bundle, rename_bundle, regread_bundle,dispatch_bundle, execute_bundle, writeback_bundle;

vector<instructionEntry> retired_instructions;

int cycle;
int counter;
vector< vector<string> > instructions;

int commit_counter = 0;
/*
 Use the above structures to initialize:
 - Reanme map table
 - Issue queue
 - Reorder buffer
 */

cycleInfo cycle_info;
instructionEntry global_inst_entry;


void init()
{
    /*
     Init rename_map_table
     The number of architectural registers is fixed to 67 (0-66)
     */
    ////////////cout << "init" << endl;
    arch_reg_num = 67;

    counter = 0;
    cycle = 1;

    head = 0;
    tail = 0;

    last_wakeup_list.clear();
    
    decode_bundle.clear();
    rename_bundle.clear();
    regread_bundle.clear();
    dispatch_bundle.clear();
    execute_bundle.clear();
    writeback_bundle.clear();
    
    renameMapTableEntry rnm_entry;
    rnm_entry.valid = 0;
    rnm_entry.rob_entry = -1;
    for (int i = 0; i < arch_reg_num; i++)
    {
        rnm_table.push_back(rnm_entry);
    }


    cycle_info.start_cycle = -1;
    cycle_info.cycle_num = -1;

    global_inst_entry.pc = -1;
    global_inst_entry.type = -1;
    global_inst_entry.des = -2;
    global_inst_entry.src1 = -2;
    global_inst_entry.src2 = -2;
    global_inst_entry.timer = -1;
    global_inst_entry.rob_des = -2;
    global_inst_entry.rob_src1 = -2;
    global_inst_entry.rob_src2 = -2;
    global_inst_entry.src1_ready = -1;
    global_inst_entry.src2_ready = -1;

    global_inst_entry.FE = cycle_info;
    global_inst_entry.DE = cycle_info;
    global_inst_entry.RN = cycle_info;
    global_inst_entry.RR = cycle_info;
    global_inst_entry.DI = cycle_info;
    global_inst_entry.IS = cycle_info;
    global_inst_entry.EX = cycle_info;
    global_inst_entry.WB = cycle_info;
    global_inst_entry.RT = cycle_info;
    
    /*
     Init iq_table, number of entries are iq_size
     */
    issueQueueEntry iq_entry;
    iq_entry.valid = 0;
    iq_entry.age = 0;
    iq_entry.inst_entry = global_inst_entry;

    for (int i = 0; i < iq_size; i++)
    {
        iq_table.push_back(iq_entry);
    }
    
    /*
     Init rob_table, number of entries are rob_size
     */
    reorderBufferEntry rob_entry;
    rob_entry.ready = -1;
    rob_entry.des_reg = -2;
    rob_entry.inst_entry = global_inst_entry;

    for (int i = 0; i < rob_size; i++)
    {
        rob_table.push_back(rob_entry);
    }
}

/*
 retire()
 - Retire up to WIDTH consecutive “ready” instructions from the head of the ROB.
 */

void retire()
{
    /*
     Retire width number of instructions at most. Stop once there is a non-ready instruction. Always retire instruction in program order.
     
     Use ready == -1 to check if head meets tail.
     */
    
    //cout << "retire" << endl;
    for (int i = 0; i < width; i++)
    {
        //cout << "head " << head << endl;
        //cout << "ready " << rob_table[head].ready << endl;
        if (rob_table[head].ready == 1)
        {   
            //Invalid the corresponding entry in rnm_table if an instruction is retired. Check if the corresponding entry has the same rob_entry value, if not, do not invalidate but continue to retire next instruction.
            //head->des_reg != -1 means the rob entry is a branch instruction
            if (rob_table[head].des_reg != -1 && rnm_table[rob_table[head].des_reg].rob_entry == head)
            {
                rnm_table[rob_table[head].des_reg].valid = 0;
            }
            
            // invalidate the entry when retire
            rob_table[head].ready = -1;
            rob_table[head].des_reg = -2;

            rob_table[head].inst_entry.RT.cycle_num = cycle - rob_table[head].inst_entry.RT.start_cycle;

            retired_instructions.push_back(rob_table[head].inst_entry);

            //cout << "retired_instructions length " << retired_instructions.size() << endl;
            
            // circular structure
            head == (rob_table.size() -1) ? head = 0 : head++;

            commit_counter++;
            //cout << "commit counter: " << commit_counter << endl;

            //cout << "retire an instruction" << endl;
        }
        else
        {
            break;
        }
    }
}


/*
 Process the writeback bundle in WB: For each instruction in WB, mark the instruction as “ready” in its entry in the ROB.
 */
void writeback()
{
    // assuming all instruciton will have a rob_des
    ////////////cout << "writeback" << endl;
    for (int i = 0; i < writeback_bundle.size(); i++)
    {   
        //cout << "writeback" << endl;
        //cout << "rob des writeback " << writeback_bundle[i].rob_des << endl;

        writeback_bundle[i].WB.cycle_num = cycle - writeback_bundle[i].WB.start_cycle;
        writeback_bundle[i].RT.start_cycle = cycle;

        rob_table[writeback_bundle[i].rob_des].inst_entry = writeback_bundle[i];
        rob_table[writeback_bundle[i].rob_des].ready = 1;
        //cout << "writeback " << endl;
    }
    
    // clear writeback_bundle after entries are pushed to reorder buffer
    //////////cout << "write bundle clear" << endl;
    writeback_bundle.clear();
}

/*
 From the execute_list, check for instructions that are finishing execution this cycle, and:
 1) Remove the instruction from the execute_list.
 2) Add the instruction to WB.
 3) Wakeup dependent instructions (set their source operand ready flags) in the IQ, DI (dispatch bundle), and RR (the register-read bundle).
 */
void execute()
{
    ////////////cout << "execute" << endl;
    last_wakeup_list.clear();
    vector<instructionEntry> tmp_bundle;

    ////cout << "execute_bundle.size()" << execute_bundle.size() << endl;
    for (int i = 0; i < execute_bundle.size(); i++)
    {
        //cout << "timer " << execute_bundle[i].timer << endl;
        if (execute_bundle[i].timer == 1)
        {   
            //cout << "execute" << endl;
            execute_bundle[i].EX.cycle_num = cycle - execute_bundle[i].EX.start_cycle;
            execute_bundle[i].WB.start_cycle = cycle;

            writeback_bundle.push_back(execute_bundle[i]);
    
            if (execute_bundle[i].rob_des != -1) last_wakeup_list.push_back((execute_bundle[i].rob_des));
        }
        else
        {
            //cout << "timer " << execute_bundle[i].timer << endl;
            execute_bundle[i].timer--;
            tmp_bundle.push_back(execute_bundle[i]);
        }
    }
    
    // wake up dependent instructions
    
    
    // clear execute_bundle, preserve the elements that are not finished yet
    //////////cout << "execute bundle clear" << endl;
    execute_bundle.clear();
    for (int i = 0; i < tmp_bundle.size(); i++)
    {
        execute_bundle.push_back(tmp_bundle[i]);
    }
    tmp_bundle.clear();
}


/*
 Issue up to WIDTH oldest instructions from the IQ. (One approach to implement oldest-first issuing is to make multiple passes through the IQ, each time finding the next oldest ready instruction and then issuing it. One way to annotate the age of an instruction is to assign an incrementing sequence number to each instruction as it is fetched from the trace file.)
 To issue an instruction:
 1) Remove the instruction from the IQ.
 2) Add the instruction to the execute_list. Set a timer for the instruction in the execute_list that will allow you to model its execution latency.
 */

struct readyEntry
{
    int age;
    int order;
};

struct comparator
{
    inline bool operator() (const readyEntry &struct1, const readyEntry &struct2)
    {
        return (struct1.age > struct2.age);
    }
};

void issue()
{
    // respond to the wakeup signal, get ready for next cycle
    for (int s = 0; s < last_wakeup_list.size(); s++)
    {
        for (int i = 0; i < iq_size; i++)
        {
            if (iq_table[i].valid == 1)
            {
                if (iq_table[i].inst_entry.rob_src1 == last_wakeup_list[s])
                {
                    iq_table[i].inst_entry.src1_ready = 1;
                }
                    
                if (iq_table[i].inst_entry.rob_src2 == last_wakeup_list[s])
                {
                    iq_table[i].inst_entry.src2_ready = 1;
                }
            }
        }
    }
    
    // search for ready instructions to issue
    vector<readyEntry> ready_list;
    for (int i = 0; i < iq_size; i++)
    {
        if (iq_table[i].valid == 1)
        {
            // increment the age of each valid instruction
            iq_table[i].age++;

            // cout << "valid" << endl;
            // cout << "src1 ready " << iq_table[i].src1_ready << endl;
            // cout << "src2 ready " << iq_table[i].src2_ready << endl;
            if (iq_table[i].inst_entry.src1_ready == 1 && iq_table[i].inst_entry.src2_ready == 1)
            {    
                readyEntry ready_entry;
                ready_entry.age = iq_table[i].age;
                ready_entry.order = i;
                ready_list.push_back(ready_entry);
                //cout << "new ready entry" << endl;
            }
        }
    }
    
    // sort ready instructions on their age
    stable_sort(ready_list.begin(), ready_list.end(), comparator());
    // cout << "ready list size " << ready_list.size() << endl;
    
    // for (int i = 0; i < ready_list.size(); i++)
    // {
    //     //cout << "valid " << ready_list[i].iq_entry.valid << endl;
    //     //cout << "type " << ready_list[i].iq_entry.type << endl;
    //     //cout << "age " << ready_list[i].iq_entry.age << endl;

    //     //cout << "order " << ready_list[i].order << endl;
    //     //cout << endl;      
    // }
    // //cout << "one round" << endl;


    // issue at most width oldest instructions
    for (int i = 0; i < width; i++)
    {
        if (i < ready_list.size())
        {   
            //cout << "issue" << endl;

            int entry_num = ready_list[i].order;

            iq_table[entry_num].inst_entry.IS.cycle_num = cycle - iq_table[entry_num].inst_entry.IS.start_cycle;
            iq_table[entry_num].inst_entry.EX.start_cycle = cycle;

            //cout << "iq_table[entry_num].inst_entry " << iq_table[entry_num].inst_entry.rob_des << endl;
            execute_bundle.push_back(iq_table[entry_num].inst_entry);

            // invalid the removed entry in iq_table
            iq_table[entry_num].valid = 0;
            iq_table[entry_num].age = 0;
        }
    }
}



/*
 If DI contains a dispatch bundle: If the number of free IQ entries is less than the size of the dispatch bundle in DI, then do nothing. If the number of free IQ entries is greater than or equal to the size of the dispatch bundle in DI, then dispatch all instructions from DI to the IQ.
 */
void dispatch()
{
    // respond to wakeup signal
    ////////////cout << "dispatch" << endl;
    for (int s = 0; s < last_wakeup_list.size(); s++)
    {
        for (int i = 0; i < dispatch_bundle.size(); i++)
        {
            if (dispatch_bundle[i].rob_src1 == last_wakeup_list[s])
            {
                dispatch_bundle[i].src1_ready = 1;
            }

            if (dispatch_bundle[i].rob_src2 == last_wakeup_list[s])
            {
                dispatch_bundle[i].src2_ready = 1;
            }
        }
    }
    
    vector<int> empty_list;
    for (int i = 0; i < iq_size; i++)
    {
        if (iq_table[i].valid == 0) empty_list.push_back(i);
    }
    
    // If the number of free IQ entries is less than the size of the dispatch bundle in DI, then do nothing. If the number of free IQ entries is greater than or equal to the size of the dispatch bundle in DI, then dispatch all instructions from DI to the IQ.
    ////cout << "empty_list size " << empty_list.size() << endl;
    if (empty_list.size() >= dispatch_bundle.size())
    {
        for (int i = 0; i < dispatch_bundle.size(); i++)
        {
            issueQueueEntry iq_entry;
            iq_entry.valid = 1;
            iq_entry.age = 0;

            dispatch_bundle[i].DI.cycle_num = cycle - dispatch_bundle[i].DI.start_cycle;
            dispatch_bundle[i].IS.start_cycle = cycle;

            iq_entry.inst_entry = dispatch_bundle[i];
            
            iq_table[empty_list[i]] = iq_entry;
        }
        
        // clear dispatch_bundle after clearing
        //////////cout << "dispatch bundle clear" << endl;
        dispatch_bundle.clear();
        empty_list.clear();
    }
}

/*
 If RR contains a register-read bundle:
 If DI is not empty (cannot accept a new dispatch bundle), then do // nothing. If DI is empty (can accept a new dispatch bundle), then process (see below) the register-read bundle and advance it from RR to DI.
 
 // How to process the register-read bundle:
 // Since values are not explicitly modeled, the sole purpose of the
 // Register Read stage is to ascertain the readiness of the renamed
 // source operands. Apply your learning from the class lectures/notes // on this topic.
 //
 // Also take care that producers in their last cycle of execution
 // wakeup dependent operands not just in the IQ, but also in two other
 // stages including RegRead()(this is required to avoid deadlock). See // Execute() description above.
 */
void regRead()
{
    // respond to wakeup signal
    ////////////cout << "reg read" << endl;
    for (int s = 0; s < last_wakeup_list.size(); s++)
    {
        for (int i = 0; i < regread_bundle.size(); i++)
        {
            if (regread_bundle[i].rob_src1 == last_wakeup_list[s])
            {
               regread_bundle[i].src1_ready = 1;
            }
            
            if (regread_bundle[i].rob_src2 == last_wakeup_list[s])
            {
                regread_bundle[i].src2_ready = 1;
            }
        }
    }

    for (int i = 0; i < regread_bundle.size(); i++)
    {
        if (rob_table[regread_bundle[i].rob_src1].ready == 1)
        {
            regread_bundle[i].src1_ready = 1;
        }
        
        if (rob_table[regread_bundle[i].rob_src2].ready == 1)
        {
            regread_bundle[i].src2_ready = 1;
        }
    }


    ////cout << "dispatch bundle size " << dispatch_bundle.size() << endl;
    if (dispatch_bundle.size() == 0)
    {
        for (int i = 0; i < regread_bundle.size(); i++)
        {
            //cout << "regread_bundle[i].rob_des " << regread_bundle[i].rob_des << endl;
            regread_bundle[i].RR.cycle_num = cycle - regread_bundle[i].RR.start_cycle;
            regread_bundle[i].DI.start_cycle = cycle;
            
            dispatch_bundle.push_back(regread_bundle[i]);
        }
        
        // remember to clear!
        //////////cout << "regread bundle clear" << endl;
        regread_bundle.clear();
    }
}


// If RN contains a rename bundle:
// If either RR is not empty (cannot accept a new register-read bundle) // or the ROB does not have enough free entries to accept the entire
// rename bundle, then do nothing.
// If RR is empty (can accept a new register-read bundle) and the ROB
// has enough free entries to accept the entire rename bundle, then
// process (see below) the rename bundle and advance it from RN to RR. //
// How to process the rename bundle:
// Apply your learning from the class lectures/notes on the steps for // renaming:
// (1) Allocate an entry in the ROB for the instruction,
// (2) Rename its source registers, and
// (3) Rename its destination register (if it has one).
// Note that the rename bundle must be renamed in program order.
// Fortunately, the instructions in the rename bundle are in program // order).
void rename()
{
    ////////////cout << "rename" << endl;
    int empty_rob_entry_num = -1;
    if (head == tail)
    {
        if (rob_table[head].ready == -1)
        {
            empty_rob_entry_num = rob_table.size();
        }
        else
        {
            empty_rob_entry_num = 0;
        }
    }
    else if (head < tail)
    {
        empty_rob_entry_num = rob_table.size() - (tail - head);
    }
    else // circular
    {
        empty_rob_entry_num = head - tail;
        ////////////cout << "head order" << head->order << endl;
        ////////////cout << "tail order" << tail->order << endl;
    }
    
    
    //////cout << "emtpy rob number: " << empty_rob_entry_num << endl;
    ////cout << "regread bundle size " << regread_bundle.size() << endl;
    if (regread_bundle.size() == 0 && empty_rob_entry_num > 0 && empty_rob_entry_num >= rename_bundle.size())
    {
        for (int i = 0; i < rename_bundle.size(); i++)
        {
            
            if (rename_bundle[i].src1 != -1)
            {
                if (rnm_table[rename_bundle[i].src1].valid == 1)
                {
                    rename_bundle[i].rob_src1 = rnm_table[rename_bundle[i].src1].rob_entry;

                    if (rob_table[rename_bundle[i].rob_src1].ready == 1)
                    {
                        rename_bundle[i].src1_ready = 1;
                    }
                    else
                    {
                        rename_bundle[i].src1_ready = 0;
                    }
                }
                else
                {
                    rename_bundle[i].rob_src1 = -1;
                    rename_bundle[i].src1_ready = 1;
                }
            }
            
            if (rename_bundle[i].src2 != -1)
            {
                if (rnm_table[rename_bundle[i].src2].valid == 1)
                {
                    rename_bundle[i].rob_src2 = rnm_table[rename_bundle[i].src2].rob_entry;

                    if (rob_table[rename_bundle[i].rob_src2].ready == 1)
                    {
                        rename_bundle[i].src2_ready = 1;
                    }
                    else
                    {
                        rename_bundle[i].src2_ready = 0;
                    }
                }
                else
                {
                    rename_bundle[i].rob_src2 = -1;
                    rename_bundle[i].src2_ready = 1;
                }
            }

            
            // install rob entry
            // no matter what type of instruction, it has to be installed in rob_table

            // rob_table[tail].des_reg can be -1
            rob_table[tail].des_reg = rename_bundle[i].des;
            rob_table[tail].ready = 0;
            
            // update rnm_table if it's not a branch
            if (rename_bundle[i].des != -1)
            {
                rnm_table[rename_bundle[i].des].valid = 1;
                rnm_table[rename_bundle[i].des].rob_entry = tail;
            }
            
            rename_bundle[i].rob_des = tail;
            //cout << "tail " << rename_bundle[i].rob_des << endl;
            
            // move tail
            tail == (rob_table.size() -1) ? tail = 0 : tail++;

            rename_bundle[i].RN.cycle_num = cycle - rename_bundle[i].RN.start_cycle;
            rename_bundle[i].RR.start_cycle = cycle;

            regread_bundle.push_back(rename_bundle[i]);
        }
        
        //////////cout << "rename bundle clear" << endl;
        rename_bundle.clear() ;
    }
}


// If DE contains a decode bundle:
// If RN is not empty (cannot accept a new rename bundle), then do // nothing. If RN is empty (can accept a new rename bundle), then // advance the decode bundle from DE to RN.
void decode()
{
    ////cout << "rename bundle size " << rename_bundle.size() << endl;
    if (rename_bundle.size() == 0)
    {
        for (int i = 0; i < decode_bundle.size(); i++)
        {
            decode_bundle[i].DE.cycle_num = cycle - decode_bundle[i].DE.start_cycle;
            decode_bundle[i].RN.start_cycle = cycle;
            
            rename_bundle.push_back(decode_bundle[i]);
        }
        
        //////////cout << "decode bundle clear" << endl;
        decode_bundle.clear();
    }
}


// Do nothing if either
// (1) there are no more instructions in the trace file or
// (2) DE is not empty (cannot accept a new decode bundle).
//
// If there are more instructions in the trace file and if DE is empty // (can accept a new decode bundle), then fetch up to WIDTH
// instructions from the trace file into DE. Fewer than WIDTH
// instructions will be fetched only if the trace file has fewer than // WIDTH instructions left.
void fetch()
{
    ////////////cout << "fetch" << endl;
    ////cout << "decode bundle size " << decode_bundle.size() << endl;
    if (decode_bundle.size() == 0 && counter < instructions.size())
    {
        for (int i = 0; i < width; i++)
        {
            if (counter < instructions.size())
            {
                instructionEntry inst_entry;

                inst_entry.pc = counter;

                istringstream ss_type(instructions[counter][1]);
                ss_type >> inst_entry.type;

                istringstream ss_des(instructions[counter][2]);
                ss_des >> inst_entry.des;
                
                istringstream ss_src1(instructions[counter][3]);
                ss_src1 >> inst_entry.src1;
                
                istringstream ss_src2(instructions[counter][4]);
                ss_src2 >> inst_entry.src2;

                switch (inst_entry.type)
                {
                    case 0:
                        inst_entry.timer = 1;
                        break;
                    case 1:
                        inst_entry.timer = 2;
                        break;
                    case 2:
                        inst_entry.timer = 5;
                        break;
                }

                inst_entry.src1 == -1 ? inst_entry.src1_ready = 1 : inst_entry.src1_ready = 0;
                
                inst_entry.src2 == -1 ? inst_entry.src2_ready = 1 : inst_entry.src2_ready = 0;

                inst_entry.rob_des = -1;
                inst_entry.rob_src1 = -1;
                inst_entry.rob_src2 = -1;

                inst_entry.FE = cycle_info;
                inst_entry.DE = cycle_info;
                inst_entry.RN = cycle_info;
                inst_entry.RR = cycle_info;
                inst_entry.DI = cycle_info;
                inst_entry.IS = cycle_info;
                inst_entry.EX = cycle_info;
                inst_entry.WB = cycle_info;
                inst_entry.RT = cycle_info;

                inst_entry.FE.start_cycle = cycle - 1;
                inst_entry.FE.cycle_num = 1;

                inst_entry.DE.start_cycle = cycle;

                decode_bundle.push_back(inst_entry);
                
                counter++;
            }
            else
            {
                break;
            }
        }
    }
}

bool advanceCycle()
{
    cycle++;

    // cout << "rob head ready" << rob_table[head].ready << endl;
    // for (int i = 0; i < iq_table.size(); i++) {
    //     if (iq_table[i].valid == 1 && iq_table[i].rob_des == head) {
    //         cout << "iq entry  " << iq_table[i].rob_src1 << " " << iq_table[i].rob_src2 << " " << iq_table[i].src1_ready << " " << iq_table[i].src2_ready << endl;
    //     }
    // }

    bool empty_rob_table = true;
    for (int i = 0; i < rob_table.size(); i++)
    {
        if (rob_table[i].ready != -1) empty_rob_table = false;
    }

    bool empty_pipeline = (decode_bundle.size() == 0 && rename_bundle.size() == 0 && regread_bundle.size() == 0 && dispatch_bundle.size() == 0 && execute_bundle.size() == 0 && writeback_bundle.size() == 0);
    
    if (counter >= instructions.size() && empty_pipeline && empty_rob_table)
    {
        return false;
    }
    else
    {
        return true;
    }
}


int main(int argc, char *argv[])
{
    istringstream iss_rob_size(argv[1]);
    iss_rob_size >> rob_size;

    istringstream iss_iq_size(argv[2]);
    iss_iq_size >> iq_size;

    istringstream iss_width(argv[3]);
    iss_width >> width;

    string file_name;
    istringstream iss_file(argv[4]);
    iss_file >> file_name;

    readTraceFile(file_name, instructions);
    
    init();
    
    do {
        retire();
        writeback();
        execute();
        issue();
        dispatch();
        regRead();
        rename();
        decode();
        fetch();
        //cout << "counter: " << counter << endl;
        //cout << "cycle: " << cycle << endl;
    } while (advanceCycle());
    
    for (int i = 0; i < retired_instructions.size(); i++)
    {
        cout << retired_instructions[i].pc << " " << "fu"<< "{" << retired_instructions[i].type << "}" << " " << "src" << "{" << retired_instructions[i].src1 << "," << retired_instructions[i].src2 << "}" << " " <<  "dst"<< "{" << retired_instructions[i].des << "}" << " " << "FE" << "{" << retired_instructions[i].FE.start_cycle << "," << retired_instructions[i].FE.cycle_num << "}" << " " << "DE" << "{" << retired_instructions[i].DE.start_cycle << "," << retired_instructions[i].DE.cycle_num << "}" << " " << "RN" << "{" << retired_instructions[i].RN.start_cycle << "," << retired_instructions[i].RN.cycle_num << "}" << " " << "RR" << "{" << retired_instructions[i].RR.start_cycle << "," << retired_instructions[i].RR.cycle_num << "}" << " " << "DI" << "{" << retired_instructions[i].DI.start_cycle << "," << retired_instructions[i].DI.cycle_num << "}" << " " << "IS" << "{" << retired_instructions[i].IS.start_cycle << "," << retired_instructions[i].IS.cycle_num << "}" << " " << "EX" << "{" << retired_instructions[i].EX.start_cycle << "," << retired_instructions[i].EX.cycle_num << "}" << " " << "WB" << "{" << retired_instructions[i].WB.start_cycle << "," << retired_instructions[i].WB.cycle_num << "}" << " " << "RT" << "{" << retired_instructions[i].RT.start_cycle << "," << retired_instructions[i].RT.cycle_num << "}" << endl;
    }

    cout << "# === Simulator Command =========" << endl;
    cout << "# ./sim_ds " << rob_size << " " << iq_size << " " << width << " " << file_name << endl;
    cout << "# === Processor Configuration ===" << endl;
    cout << "# ROB_SIZE = " << rob_size << endl;
    cout << "# IQ_SIZE  = " << iq_size << endl;
    cout << "# WIDTH    = " << width << endl;
    cout << "# === Simulation Results ========" << endl;
    cout << "# Dynamic Instruction Count      = " << counter << endl;
    cout << "# Cycles                         = " << cycle - 1 << endl;

    cout << "# Instructions Per Cycle (IPC)   = " << roundf(double(counter)/double(cycle) * 100) / 100 << endl;


    return 0;
}


