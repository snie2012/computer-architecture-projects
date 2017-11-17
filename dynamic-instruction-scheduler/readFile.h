#ifndef readFile_h
#define readFile_h


#include <fstream>
#include <sstream>

#include "common.h"


// Declaration
void readTraceFile(string &file_name, vector< vector<string> > &instructions);

void readTraceFile(string &file_name, vector< vector<string> > &instructions) 
/*
Assuming file is structured in the following way:
1. Line by line, no empty line
2. Every line is in this format: ab120024 0 1 2 3

@file_name: string

Return format:
- a vector of vector of strings, such as [['hex', '0', '1', '2', '3'], ['hex', '0', '-1', '2', '54']]
*/
{
    ifstream file(file_name.c_str());

    string line;
    string token;

    while (getline(file, line))
    {   
        istringstream iss(line);

        vector<string> inst;
        while (getline(iss, token, ' '))
        {
            inst.push_back(token);
        }
        instructions.push_back(inst);
    };
}


#endif





