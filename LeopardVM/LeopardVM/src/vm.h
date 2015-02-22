#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <list>

#include "classinfo.h"
#include "main.h"
#include "jit.h"
#include "allocatedObject.h"
#include "gc.h"

#ifndef __VM_H__
#define __VM_H__

using namespace std;


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
        
        static unsigned long allocateArray (char* type, int size);
        static unsigned long allocateObject (char *type);
        static void callMethod (vector<VariableDescriptor*>* vectorVarDesc, ClassInfo *classInfo, MethodInfo *methodInfo);
        ClassInfo *getClassInfoForName (string name);
        AllocatedObject *getAllocatedObjectForStartPos (unsigned long startPos);
        unsigned int getPosForField (string cname, string fname, MemberInfo** type);
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

        AllocatedVariable* getAllocObjForStaticMember (MemberInfo *meminfo)
        {
            return mapStaticMembersAllocated.at (meminfo);
        }

        GarbageCollector* getGarbageCollector ()
        {
            return &gc;
        }

        vector<AllocatedVariable *>* getRootSet ();

    private:
        bool isLittleEndian;
        string mainFunction;
        string mainClass;
        vector<ClassInfo*> vectorClassInfo;
        map<unsigned long, AllocatedObject*> mapAllocatedObject;
        map<MemberInfo*, AllocatedVariable*> mapStaticMembersAllocated;
        vector<MemberInfo*> vectorStaticMembers;
        stack<JIT*> stackJIT;
        JIT* jit;
        HeapAllocator* heapAllocator;
        unsigned long calledObjectAddressMem;
        unsigned long returnValueMem;
        GarbageCollector gc;
        vector<AllocatedVariable *> rootSet;
        
        int read (const string filename);
        int getSizeForType (char *type);
        AllocatedObject* _allocateObject (string type);
        AllocatedVariable* allocateStaticMember (ClassInfo *classInfo, 
                                               MemberInfo *memberInfo);
};

#endif /* __VM_H__ */