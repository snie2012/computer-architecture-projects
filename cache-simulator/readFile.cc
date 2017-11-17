#include "readFile.h"

#include <fstream>
#include <sstream>

//reference http://stackoverflow.com/questions/18310952/convert-strings-between-hex-format-and-binary-format
const char* hexCharToBin(char c) {
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

string hexStrToBinStr(const string& hex_str) {
    
    string bin_str;
    
    for(unsigned i = 0; i < hex_str.length(); ++i)
    {
       bin_str += hexCharToBin(hex_str[i]);
    }

    return bin_str;
}


vector<string> readTraceFile(string file_name) {
    ifstream file(file_name.c_str());
    string line;
    vector<string> memory_trace;
    while (getline(file, line))
    {   
        memory_trace.push_back(line.substr(0, 1) + " " +  hexStrToBinStr(line.substr(2, line.size()-1)));
    };

    return memory_trace;
}





