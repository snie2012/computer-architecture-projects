#include "util.h"

namespace util {

int binToDec(const char * bin_str)
{
    int dec = 0;
     
    while (*bin_str != '\0')
        dec = 2 * dec + (*bin_str++ - '0');
    return dec;
}

}