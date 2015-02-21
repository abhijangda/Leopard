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
    
        byte *getMemory ()
        {
            return memory;
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

class AllocatedObject
{
    public:
        AllocatedObject (ClassInfo *_classInfo, MemoryBlock* _memBlock)
        {
            classInfo = _classInfo;
            memBlock = _memBlock;
        }

        void addChild (AllocatedObject *child)
        {
            listChildren.insert (listChildren.end (), child);
        }
        
        AllocatedObject* getChild (int i)
        {
            return listChildren[i];
        }
    
        MemoryBlock* getMemBlock () const
        {
            return memBlock;
        }
    
        ClassInfo* getClassInfo ()
        {
            return classInfo;
        }

    private:
        ClassInfo *classInfo;
        vector<AllocatedObject*> listChildren;
        MemoryBlock *memBlock;
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
        
        static unsigned long allocateArray (char* type, int size);
        static unsigned long allocateObject (char *type);
        static void callMethod (vector<VariableDescriptor*>* vectorVarDesc, ClassInfo *classInfo, MethodInfo *methodInfo);
        ClassInfo *getClassInfoForName (string name);
        AllocatedObject *getAllocatedObjectForStartPos (unsigned long startPos);
        unsigned int getPosForField (string cname, string fname, string* type);
        MethodInfo *getMethodInfoOfClass (string cname, string mname);
        
        JIT *getCurrentJIT ()
        {
            return jit;
        }

        JIT *pushJIT ()
        {
            jit = new JIT ();
            stackJIT.push (jit);
            return jit;
        }
    
        JIT *popJIT ()
        {
            JIT* jit2 = stackJIT.top ();
            stackJIT.pop ();
            jit = stackJIT.top ();
            return jit2;
        }

        unsigned long *getcalledObjectAddressMem ()
        {
            return &calledObjectAddressMem;
        }
    
        unsigned long *getReturnValueMem ()
        {
            return &returnValueMem;
        }

    private:
        bool isLittleEndian;
        string mainFunction;
        string mainClass;
        vector<ClassInfo*> vectorClassInfo;
        map<unsigned long, AllocatedObject*> mapAllocatedObject; 
        stack<JIT*> stackJIT;
        JIT* jit;
        HeapAllocator* heapAllocator;
        unsigned long calledObjectAddressMem;
        unsigned long returnValueMem;
        
        int read (const string filename);
        int getSizeForType (char *type);
        AllocatedObject* _allocateObject (string type);
};

#endif /* __VM_H__ */