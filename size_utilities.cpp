#include "size_utilities.hpp"

FileSizePrefix sizePrefix(unsigned long long i){
    if(i<1000u) return fsp_B;
    else if(i<1000000u) return fsp_KB;
    else if(i<1000000000ull) return fsp_MB;
    else if(i<1000000000000ull) return fsp_GB;
    else return fsp_TB; // Unlikely :)
}

char sizePrefixChar(unsigned long long i){
    static const char lookup_table_[] = {' ', 'K', 'M', 'G', 'T'};
    return lookup_table_[sizePrefix(i)];
}

const char *sizeNumberString(char buffer[8], unsigned long long i){
    unsigned char at = 0;
    static const unsigned long long lookup_table_[] = {1u, 100u, 100000u, 100000000ull, 100000000000ull};
    unsigned long long p = i/lookup_table_[sizePrefix(i)];
    if(p>=1000)
        buffer[at++] = ((p/1000)%10)+'0';
    if(p>=100)
        buffer[at++] = ((p/100)%10)+'0';
    if(p>=10)
        buffer[at++] = ((p/10)%10)+'0';
    buffer[at++] = '.';
    buffer[at++] = (p%10)+'0';
    buffer[at++] = 0;

    return buffer;
}
