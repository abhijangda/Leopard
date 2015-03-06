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

class HeapPartition
{
    private:
        map<unsigned long, MemoryBlock*> blockLists;
        list<MemoryBlock*> freeBlockLists;
        list<MemoryBlock*>::iterator iterCurrBlock; //Last position where block was added
        MemoryBlock *createNewBlock (unsigned long size);
        byte* allocatedMemory;
        unsigned long sizeFilled;
        MemoryBlock *lastBlock;

    public:
        MemoryBlock *allocate (unsigned long size);
        int id;

        ~HeapPartition ()
        {
            delete allocatedMemory;
        }
    
        unsigned long getStartAddress ()
        {
            return (unsigned long)allocatedMemory;
        }

        HeapPartition (int id);
        
        void freeMemory (unsigned long address);
        vector<AllocatedObject*>* getReachableObjects ();
};

class HeapAllocator
{
    private:
        /* We don't have to delete any partition. Insertion at the
         * end of a vector is O(1). While we have to search through
         * partitions to get to which partition a certain address
         * belongs, for this purpose map is good */
        list<ReferenceVariable*> listReferenceVars;
        vector<HeapPartition*> vectorPartitions;
        map<unsigned long, HeapPartition*> mapPartitions;

    public:
        HeapAllocator ();
        ~HeapAllocator ()
        {
            for (int i = 0; i < vectorPartitions.size (); i++)
            {
                delete vectorPartitions.at (i);
            }
        }
        
        HeapPartition* getPartitionWithAddress (unsigned long address);
        MemoryBlock *allocate (unsigned long size);
        MemoryBlock* allocateInPartition (int partition, unsigned long size);
        vector<AllocatedObject*>* reachableObjectsForPartition (int partitionNum);
        void freeAddress (unsigned long address);
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
        static unsigned long allocateObject (vector<VariableDescriptor*>* stackVarDesc, char *type);
        static void callMethod (vector<VariableDescriptor*>* vectorVarDesc, 
                                vector<VariableDescriptor*>* stackVarDesc, 
                                ClassInfo *classInfo, MethodInfo *methodInfo);
        ClassInfo *getClassInfoForName (string name);
        AllocatedObject *getAllocatedObjectForStartPos (unsigned long startPos);
        unsigned int getPosForField (string cname, string fname, MemberInfo** type);
        MethodInfo *getMethodInfoOfClass (string cname, string mname);
        void updateAddressForAllocatedObject (unsigned long prevAddress, 
                                              unsigned long newAddress, AllocatedObject* obj)
        {
            mapAllocatedObject.erase (prevAddress);
            mapAllocatedObject [newAddress] = obj;
        }
    
        JIT *getCurrentJIT ()
        {
            return jit;
        }

        JIT *pushJIT (JIT* to_push = LULL)
        {
            if (to_push == LULL)
            {
                jit = new JIT ();
            }
            else
            {
                jit = to_push;
            }

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

        vector<VariableDescriptor*>* getAllStackVariables ();
        
        void pushCallerStack (vector<VariableDescriptor*>* stackVarDesc)
        {
            vectorStackVarDesc.push_back (stackVarDesc);
        }

        void popCallerStack ()
        {
            vectorStackVarDesc.pop_back ();
        }
        
        RootSet* getRootSet ();
        void getReachableForAddress (unsigned long address,
                                     vector<AllocatedObject*>& vec);
        AllocatedObject* getAllocObjectForAddress (unsigned long address)
        {
            return mapAllocatedObject.at (address);
        }
    
        void markAllObjectsUnreachable ();
        void doGarbageCollection ();
        JIT* getJITForMethodInfo (MethodInfo* methodInfo)
        {
            return mapJITForMethod.at (methodInfo);
        }

        void addJITForMethodInfo (MethodInfo* methodInfo, JIT* jit)
        {
            mapJITForMethod [methodInfo] = jit;
        }
        
    private:
        bool isLittleEndian;
        string mainFunction;
        string mainClass;
        vector<ClassInfo*> vectorClassInfo;
        map<unsigned long, AllocatedObject*> mapAllocatedObject;
        map <MethodInfo*, JIT*> mapJITForMethod;
        map<MemberInfo*, AllocatedVariable*> mapStaticMembersAllocated;
        vector<MemberInfo*> vectorStaticMembers;
        stack<JIT*> stackJIT;
        JIT* jit;
        HeapAllocator* heapAllocator;
        unsigned long calledObjectAddressMem;
        unsigned long returnValueMem;
        GarbageCollector gc;
        vector<vector<VariableDescriptor*>*> vectorStackVarDesc;

        int read (const string filename);
        int getSizeForType (char *type);
        AllocatedObject* _allocateObject (vector<VariableDescriptor*>* stackVarDesc, string type);
        AllocatedVariable* allocateStaticMember (ClassInfo *classInfo, 
                                               MemberInfo *memberInfo);
};

#endif /* __VM_H__ */