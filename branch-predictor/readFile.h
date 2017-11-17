#ifndef readFile_h
#define readFile_h


#include <fstream>
#include <sstream>

#include "common.h"


// Declaration
vector<string> readTraceFile(string file_name);


// Implementation
const char* hexCharToBin(char c) 
/*
//reference http://stackoverflow.com/questions/18310952/convert-strings-between-hex-format-and-binary-format
*/
{
    switch(toupper(c))
    {
        case '0': return "0000";
        case '1': return "0001";
        case '2': return "0010";
        case '3': return "0011";
        case '4': return "0100";
        case '5': return "0101";
        case '6': return "0110";
        case '7': return "0111";
        case '8': return "1000";
        case '9': return "1001";
        case 'A': return "1010";
        case 'B': return "1011";
        case 'C': return "1100";
        case 'D': return "1101";
        case 'E': return "1110";
        case 'F': return "1111";
    }
}

string binToHex(string bin_str) 
/*
One to one corresponding relationship between binary and hexadecimal
*/
{
    // Unable to use switch statement here, since switch doesn't support string comparison.
    // Consider using a hash to indirectly using switch: http://stackoverflow.com/questions/650162/why-switch-statement-cannot-be-applied-on-strings
    if (bin_str == "0000") return "0";
    if (bin_str == "0001") return "1";
    if (bin_str == "0010") return "2";
    if (bin_str == "0011") return "3";
    if (bin_str == "0100") return "4";
    if (bin_str == "0101") return "5";
    if (bin_str == "0110") return "6";
    if (bin_str == "0111") return "7";
    if (bin_str == "1000") return "8";
    if (bin_str == "1001") return "9";
    if (bin_str == "1010") return "a";
    if (bin_str == "1011") return "b";
    if (bin_str == "1100") return "c";
    if (bin_str == "1101") return "d";
    if (bin_str == "1110") return "e";
    if (bin_str == "1111") return "f";
}


string binStrToHexStr(string bin_str)
/*
Convert a binary string to a hex string.
@bin_str: Always check first if the length of bin_str is divisible by 4, if it is not, add 0s to the higher bits to make the length divisible by 4
*/
{
    string result;

    unsigned modulus = bin_str.length() % 4;

    if (modulus != 0)
    {
        for (unsigned i = 0; i < 4 - modulus; i++)
        {
            bin_str = "0" + bin_str;
        }
    }

    for (unsigned i = 0; i < bin_str.length(); i += 4)
    {
        result = result + binToHex(bin_str.substr(i, 4));
    }

    return result;
}




string hexStrToBinStr(const string& hex_str) 
/*
Transform hex string to binary string

@hex_str: string

@return: string
*/
{    
    string bin_str;
    
    for(unsigned i = 0; i < hex_str.length(); ++i)
    {
       bin_str += hexCharToBin(hex_str[i]);
    }

    return bin_str;
}


vector<string> readTraceFile(string file_name) 
/*
Assuming file is structured in the following way:
1. Line by line, no empty line
2. Every line has a memory address in hex format and an access type of the address, seperated by a space

@file_name: string

@return: vector<string>
*/
{
    ifstream file(file_name.c_str());
    string line;
    vector<string> memory_trace;

    while (getline(file, line))
    {   
        memory_trace.push_back(line.substr(line.size()-1, 1) + " " +  hexStrToBinStr(line.substr(0, line.size()-2)));
    };

    return memory_trace;
}


#endif