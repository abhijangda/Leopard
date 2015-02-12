#include <string>

#include "vm.h"

#ifndef __HELPER_FUNCTIONS_H__
#define __HELPER_FUNCTIONS_H__

using namespace std;

string char_to_string (char c);
int convert_byte_array_to_integer (byte array[], bool isLittleEndian);
string read_string_from_byte_array (byte array[], int len);
long convert_byte_array_to_long (byte array[], bool isLittleEndian);
int convert_string_to_int (string s);
#endif /* __HELPER_FUNCTIONS_H__ */