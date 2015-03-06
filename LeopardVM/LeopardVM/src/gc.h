#include "main.h"
#include "allocatedObject.h"
#include "jit.h"

#include <vector>

#ifndef __GC_H__
#define __GC_H__

typedef vector<AllocatedObject*> Partition;

class RootSet
{
    private:
        stack<JIT*>* stackJIT;
        vector<vector<VariableDescriptor*>*>* vectorVectorVarDesc;
        vector<AllocatedObject*>* staticMembers;
    
    public:
        RootSet (vector<vector<VariableDescriptor*>*>* _vectorVectorVarDesc, 
                 stack<JIT*>* _stackJIT, vector<AllocatedObject*>* _staticMembers)
        {
            stackJIT = _stackJIT;
            vectorVectorVarDesc = _vectorVectorVarDesc;
            staticMembers = _staticMembers;
        }
        
        int getJITCount ()
        {
            return stackJIT->size ();
        }

        vector<VariableDescriptor*>* getJITStackVariables (int jitNo)
        {
            //,return vectorVectorVarDesc [jitNo];
        }

        vector<AllocatedObject*>* getRootAllocatedObjects ();
    
        ~RootSet ()
        {
            /* Delete stackJIT and staticMembers only as vectorVectorVarDesc is allocated by
             * VirtualMachine.
             */
            delete stackJIT;
            delete staticMembers;
        }
};

class GarbageCollector
{
    public:
        void collectGarbage ();
        GarbageCollector ();
        ~GarbageCollector ();
        void pushObject (AllocatedObject* object);

    private:
        vector <Partition*> vectorPartitions;
        unsigned long sizeOfPartition;
        void isPartitionFull (int partitionNum);
        void setChildrenReachable (AllocatedObject *allocObj);
        void setReachableObjects ();
        void collectPartition (int partitionNum);
};

#endif /* __GC_H__ */