#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <list>

#include "classinfo.h"
#include "main.h"
#include "jit.h"

#ifndef __VM_H__
#define __VM_H__

using namespace std;

class MemoryBlock
{
    private:
        unsigned long size; //Size is in Bytes
        unsigned long startPos;
        byte *memory;
    
    public:
        MemoryBlock (unsigned long size, unsigned long startPos, byte *memory)
        {
            this->size = size;
            this->startPos = startPos;
            this->memory = memory;
        }
    
        MemoryBlock (unsigned long size)
        {
            this->memory = new byte[size];
            this->size = size;
            this->startPos = (unsigned long)this->memory;
        }
        
        unsigned long getStartPos ()
        {
            return startPos;
        }
};

class ReferenceVariable
{
    private:
        MemoryBlock* memBlock;
};

class HeapAllocator
{
    private:
        list<MemoryBlock*> blockLists;
        list<MemoryBlock*> freeBlockLists;
        list<MemoryBlock*>::iterator iterCurrBlock; //Last position where block was added
        list<ReferenceVariable*> listReferenceVars;
        MemoryBlock *createNewBlock (unsigned long size);

    public:
        HeapAllocator ();
        ~HeapAllocator ()
        {
        }
        
        MemoryBlock *allocate (unsigned long size);
};

class VirtualMachine 
{
    public:
        int start (string filename);
        VirtualMachine ()
        {
            jit = new JIT ();
            heapAllocator = new HeapAllocator ();
        }
        
        HeapAllocator *getHeapAllocator ()
        {
            return heapAllocator;
        }
        
        static long allocateArray (char* type, int size);

    private:
        bool isLittleEndian;
        string mainFunction;
        string mainClass;
        vector<ClassInfo*> vectorClassInfo;
        JIT* jit;
        HeapAllocator* heapAllocator;
        
        int read (const string filename);
        int getSizeForType (char *type);
};

#endif /* __VM_H__ */