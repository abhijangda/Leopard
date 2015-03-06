#include "gc.h"
#include "vm.h"

#include <stdio.h>

extern VirtualMachine* ptrVM;

vector<AllocatedObject*>* RootSet::getRootAllocatedObjects ()
{
    int vecVecIter;
    AllocatedObject *obj;
    vector<AllocatedObject*>* vecAllocObj;
    
    vecAllocObj = new vector<AllocatedObject*> ();
    vecVecIter = vectorVectorVarDesc->size () - 1;

    /* First add all locals and temporaries of current stacks */
    while (stackJIT->size () != 0 && vecVecIter >= 0)
    {
        vector<LocalDescriptor*> vectorLocalDesc;
        AllocatedObject* allocObj;

        vectorLocalDesc = *stackJIT->top()->getLocalDescriptors ();

        for (int i = 0; i < vectorLocalDesc.size (); i++)
        {
            if (getOperatorTypeFromString (vectorLocalDesc[i]->getType ()) != 
                Reference)
            {
                continue;
            }

            unsigned long *r;
            r = (unsigned long *)(vectorLocalDesc[i]->getMemLoc () + *stackJIT->top()->getStackPointerMem ());

            if ((*r) != 0)
            {
                /* If value is 0 then address has not been assigned */
                allocObj = ptrVM->getAllocObjectForAddress (*r);
                allocObj->pushAddress (r);
                vecAllocObj->push_back (allocObj);
            }
        }

        vector<VariableDescriptor*> vectorVarDesc;
        
        vectorVarDesc = *stackJIT->top()->getArgumentDescriptors ();

        for (int i = 0; i < vectorVarDesc.size (); i++)
        {
            /* TODO: Correct for arguments. 
             * Although this shouldn't be different in any way */
        }

        vector<VariableDescriptor*>* vecVarDesc;
        
        vecVarDesc = vectorVectorVarDesc->at(vecVecIter);

        for (int i = 0; i < vecVarDesc->size (); i++)
        {
            AllocatedObject* allocObj;

            if (getOperatorTypeFromString (vecVarDesc->at(i)->getType ()) != 
                Reference)
            {
                continue;
            }

            unsigned long *r;
            r = (unsigned long *)(vecVarDesc->at(i)->getMemLoc () + *stackJIT->top()->getStackPointerMem ());
            allocObj = ptrVM->getAllocObjectForAddress (*r);
            allocObj->pushAddress (r);
            vecAllocObj->push_back (allocObj);
        }

        vecVecIter--;
        stackJIT->pop ();
    }

    /* Now add all the static members */
    for (int i = 0; i < staticMembers->size (); i++)
    {
        vecAllocObj->push_back (staticMembers->at (i));
    }

    return vecAllocObj;
}

GarbageCollector::GarbageCollector ()
{
    vectorPartitions.push_back (new Partition ());
    sizeOfPartition = 100;
}

GarbageCollector::~GarbageCollector ()
{
    for (int i = 0; i < vectorPartitions.size (); i++)
    {
        delete vectorPartitions[i];
    }
}

void GarbageCollector::pushObject (AllocatedObject* object)
{    
    isPartitionFull (0);
    vectorPartitions[0]->push_back (object);
}

void GarbageCollector::setChildrenReachable (AllocatedObject *allocObj)
{
    if (!allocObj)
    {
        return;
    }

    allocObj->setReachable ();
    
    for (int i = 0; i < allocObj->getTotalChildren (); i++)
    {
        setChildrenReachable (allocObj->getChild (i));
    }
}

void GarbageCollector::setReachableObjects ()
{
    vector<AllocatedObject*>* vecAllocObj;
    RootSet *rootSet;

    ptrVM->markAllObjectsUnreachable ();
    rootSet = ptrVM->getRootSet ();
    vecAllocObj  = rootSet->getRootAllocatedObjects ();
    
    for (int i = 0; i < vecAllocObj->size (); i++)
    {
        setChildrenReachable (vecAllocObj->at (i));
    }

    delete rootSet;
    delete vecAllocObj;
}

void GarbageCollector::collectGarbage ()
{
    setReachableObjects ();
    collectPartition (0);
}

void GarbageCollector::collectPartition (int partitionNum)
{
    int i;
    HeapAllocator* heapAllocator;
    vector<AllocatedObject*>* reachableObjects;
    
    /* Get all reachable objects of partition*/
    heapAllocator = ptrVM->getHeapAllocator ();
    reachableObjects = heapAllocator->reachableObjectsForPartition (partitionNum);
    
    /* Insert them one by one in the partition */
    for (i = 0; i < reachableObjects->size (); i++)
    {
        MemoryBlock *memBlock;
        int size;
        AllocatedObject* allocObj;

        size = reachableObjects->at (i)->getClassInfo ()->getSize ();
        memBlock = heapAllocator->allocateInPartition (partitionNum+1, size);

        if (memBlock)
        {
            MemoryBlock *prevMemBlock;
            byte* prevMem;
            byte *mem;
            int j = 0;
            
            allocObj = reachableObjects->at(i);
            prevMemBlock = allocObj->getMemBlock ();
            prevMem = prevMemBlock->getMemory ();
            mem = memBlock->getMemory ();
            ptrVM->updateAddressForAllocatedObject (prevMemBlock->getStartPos (),
                                                    memBlock->getStartPos (),
                                                    allocObj);
            memBlock->setAllocatedVariable (allocObj);
            allocObj->setMemBlock (memBlock);

            /* Copy data from previous block to the new block*/
            for (j = 0; j < memBlock->getSize (); j++)
            {
                mem[j] = prevMem[j];
            }

            heapAllocator->freeAddress (prevMemBlock->getStartPos ());

            /* Update the value of pointer in the reachable objects */
            j = allocObj->totalAddresses () - 1;

            while (j >= 0)
            {
                *allocObj->popAddress () = memBlock->getStartPos ();
                j--;
            }
        }
        else
        {
            break;
        }
    }

    if (i < reachableObjects->size ())
    {
        /* Not all objects have been allocated
         * collect this partition also */
        collectPartition (partitionNum + 1);
        
        /* Insert remaining reachable objects in this partition */
        for (; i < reachableObjects->size (); i++)
        {
            MemoryBlock *memBlock;
            int size;
            AllocatedObject* allocObj;
    
            size = reachableObjects->at (i)->getClassInfo ()->getSize ();
            memBlock = heapAllocator->allocateInPartition (partitionNum+1, size);
    
            if (memBlock)
            {
                MemoryBlock *prevMemBlock;
                int j = 0;
    
                allocObj = reachableObjects->at(i);
                prevMemBlock = allocObj->getMemBlock ();
                ptrVM->updateAddressForAllocatedObject (prevMemBlock->getStartPos (),
                                                        memBlock->getStartPos (),
                                                        allocObj);
                memBlock->setAllocatedVariable (reachableObjects->at (i));
                allocObj->setMemBlock (memBlock);
                
                /* Copy data from previous block to the new block*/
                for (j = 0; j < memBlock->getSize (); j++)
                {
                    memBlock->getMemory ()[i] = prevMemBlock->getMemory ()[i];
                }

                heapAllocator->freeAddress (prevMemBlock->getStartPos ());
    
                /* Update the value of pointer in the reachable objects */
                j = allocObj->totalAddresses () - 1;
                while (j >= 0)
                {
                    *allocObj->popAddress () = memBlock->getStartPos ();
                    j--;
                }
            }
            else
            {
                break;
            }
        }
    }

    delete reachableObjects;
}

void GarbageCollector::isPartitionFull (int partitionNum)
{
    unsigned long size = 0;
    int i = 0;

    for (i = 0; i < vectorPartitions[partitionNum]->size (); i++)
    {
        size += vectorPartitions[partitionNum][0][i]->getClassInfo ()->getSize ();
        
        if (size >= sizeOfPartition)
        {
            return;
        }
    }
}