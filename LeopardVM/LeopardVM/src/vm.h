#include <iostream>
#include <string>
#include <vector>

#include "classinfo.h"
#include "main.h"
#include "jit.h"

#ifndef __VM_H__
#define __VM_H__

using namespace std;

class VirtualMachine 
{
    public:
        int start (string filename);
        VirtualMachine ()
        {
            jit = new JIT ();
        }
           
    private:
        bool isLittleEndian;
        string mainFunction;
        string mainClass;
        vector<ClassInfo*> vectorClassInfo;
        JIT* jit;

        int read (const string filename);
};

#endif /* __VM_H__ */