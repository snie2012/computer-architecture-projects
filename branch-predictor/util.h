#ifndef util_h
#define util_h

#include "common.h"

namespace util {

// Declaration
int binToDec(const char *bin_str); 

unsigned sniePow(unsigned a, unsigned n);

string xorBinStr(string &b1, string &b2);


// Implementation
int binToDec(const char *bin_str)
/*
Transform a binary string to an int

@para: char*
@return: int
*/
{
    int dec = 0;
     
    while (*bin_str != '\0')
        dec = 2 * dec + (*bin_str++ - '0'); // *bin_str++ - '0' casts string into numerical type
    return dec;
}



/*
Use divide and conquer to calculate a to the power of n
Assuming positive integers as input
*/
unsigned sniePow(unsigned a, unsigned n)
{
    if (n == 1) 
    {
        return a;
    } 
    else 
    {
        if (n % 2 == 0)
        {
            return sniePow(a, n/2) * sniePow(a, n/2);
        }
        else 
        {
            return sniePow(a, (n-1)/2) * sniePow(a, (n-1)/2) * a;
        }
    }
}



/*
Perform XOR logic on two unit length string

Inputs: two binary string. Each string has length 1
Return: a binary string with length 1 after XOR operation
*/

string XOR(string b1, string b2)
{
    if (b1 == "0")
    {   
        if (b2 == "0") 
        {
            return "0";
        }
        else
        {
            return "1";
        }
    }
    else
    {
        if (b2 == "1") 
        {
            return "0";
        }
        else
        {
            return "1";
        }
    }
}


/*
Perform XOR operation on two binary string

Inputs: two binary string. Assuming with equal length
Return: a binary string after XOR operation
*/

string xorBinStr(string &b1, string &b2)
{
    string result = "";
    for (unsigned i = 0; i < b1.length(); i++)
    {
        result += XOR(b1.substr(i, 1), b2.substr(i, 1));
    }

    return result;
}


}

#endif































