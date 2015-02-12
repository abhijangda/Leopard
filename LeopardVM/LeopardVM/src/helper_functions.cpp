#include <sstream>
#include <stdio.h>

#include "helper_functions.h"

string char_to_string (char c)
{
    stringstream ss;
    string s;
    ss << c;
    ss >> s;
    
    return s;
}

int convert_string_to_int (string s)
{
    int i;
    istringstream (s) >> i;
    return i;
}

long convert_byte_array_to_long (byte array[], bool isLittleEndian)
{
    int i = 0;
    long ans = 0;
    
    if (isLittleEndian)
    {
        for (i = 7; i >= 0; i--)
        {
            ans = ans + (((int)array [i]) << (i*8));
        }
    }
    else
    {
        for (i = 0; i < 8; i++)
        {
            ans = ans + (((int)array [i]) << (i*8));
        }
    }

    return ans;
}

int convert_byte_array_to_integer (byte array[], bool isLittleEndian)
{
    int i = 0;
    int ans = 0;
    
    if (isLittleEndian)
    {
        for (i = 3; i >= 0; i--)
        {
            ans = ans + (((int)array [i]) << (i*8));
        }
    }
    else
    {
        for (i = 0; i < 4; i++)
        {
            ans = ans + (((int)array [i]) << (i*8));
        }
    }

    return ans;
}

string read_string_from_byte_array (byte array[], int len)
{
    string s = "";
    while (len > 0)
    {
        s = char_to_string ((char)array [len]) + s;
        len --;
    }

    return s;
}
