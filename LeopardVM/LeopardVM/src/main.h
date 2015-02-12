#include <string>

#ifndef __MAIN_H__
#define __MAIN_H__

using namespace std;

typedef unsigned char byte;

enum OperatorType
{
    SignedChar,
    UnsignedChar,
    Short,
    UnsignedShort,
    Integer,
    UnsignedInteger,
    Long,
    Float,
    Double,
    Reference,
};

typedef enum OperatorType OperatorType;

OperatorType getOperatorTypeFromString (string type);
string getStringFromOperatorType (OperatorType type);
int getSizeFromOperatorType (OperatorType type);

#endif /* __MAIN_H__ */