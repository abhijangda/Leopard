#include "vm.h"

#include <stdio.h>

#define PARTITION_SIZE 1024*1024

extern VirtualMachine *ptrVM;

/* Maintain a a list of all blocks 
 * sorted on the basis of start and ending address
 * Use First Fit to determine the free block to be assigned.
 */
HeapPartition::HeapPartition (int id)
{    
    /* Allocate the partition's memory */
    /* This will ensure that the memory allocated is
     * contiguos */
    allocatedMemory = new byte [PARTITION_SIZE];
    /* lastBlock is a dummy block of size 0. It just marks the end of memory */
    lastBlock = new MemoryBlock (0, allocatedMemory + PARTITION_SIZE);
    lastBlock->setAllocatedVariable (LULL);
    blockLists [(unsigned long)allocatedMemory + PARTITION_SIZE] = lastBlock;
    sizeFilled = 0;
    this->id = id;
}

MemoryBlock *HeapPartition::createNewBlock (unsigned long size)
{
    byte *mem = LULL;
    MemoryBlock *memBlock;

    /* Store all the MemoryBlocks in sorted order on the basis of their
     * starting position */
    /* Use First Fit to determine the block to use */

    map<unsigned long, MemoryBlock*>::iterator iter;
    unsigned long prev = (unsigned long)allocatedMemory;

    for (iter = blockLists.begin (); iter != blockLists.end (); ++iter)
    {
        if (prev != iter->second->getStartPos ())
        {
            /* Found empty space */
            unsigned long availSize = iter->second->getStartPos () - prev;
            
            if (availSize >= size)
            {
                /* Found the available area */
                mem = (byte *)prev;
                break;
            }
            else
            {
                prev = iter->second->getStartPos ();
            }
        }
        
        prev += iter->second->getSize ();
    }
    iter = blockLists.end ();
    iter--;

    if (prev == ((iter)->second->getStartPos ()))
    {
        /* ERROR: Partition Full run garbage collector */
        return LULL;
    }

    memBlock = new MemoryBlock (size, mem);
    blockLists[(unsigned long)mem] = memBlock;
    sizeFilled += size;

    return memBlock;
}

MemoryBlock *HeapPartition::allocate (unsigned long size)
{
    return createNewBlock (size);
}
 
void HeapPartition::freeMemory (unsigned long address)
{
    MemoryBlock *memBlock;

    memBlock = blockLists[address];
    blockLists.erase (address);

    delete memBlock;    
}

vector<AllocatedObject*>* HeapPartition::getReachableObjects ()
{
    vector<AllocatedObject*>* vectorAllocObj;
    map<unsigned long, MemoryBlock*>::iterator iter;
    vector<unsigned long> vectorToBeFreed;

    vectorAllocObj = new vector<AllocatedObject*> ();

    /* Remember there is a dummy memory block at the end of
     * map used as an end marker */
    for (iter = blockLists.begin (); iter != blockLists.end (); ++iter)
    {
        AllocatedObject *allocObj;
        
        if (iter->second->getAllocatedVariable () == LULL)
        {
            continue;
        }

        allocObj = dynamic_cast<AllocatedObject*> (iter->second->getAllocatedVariable ());

        if (allocObj->getIsReachable ())
        {
            vectorAllocObj->push_back (allocObj);
        }
        else
        {
            /* Not reachable so freeeeee */
            vectorToBeFreed.push_back (allocObj->getMemBlock ()->getStartPos ());
        }
    }

    for (int l = 0; l < vectorToBeFreed.size (); l++)
    {
        freeMemory (vectorToBeFreed[l]);
    }

    return vectorAllocObj;
}

HeapAllocator::HeapAllocator ()
{
    vectorPartitions.push_back (new HeapPartition (vectorPartitions.size ()));
}

MemoryBlock* HeapAllocator::allocateInPartition (int partition, 
                                                 unsigned long size)
{
    MemoryBlock* memBlock;

    memBlock = vectorPartitions.at (partition)->allocate (size);

    if (memBlock != LULL)
    {
        return memBlock;
    }

    /* Partition is full */
    /* Access higher level partition 
     * Create it if not created */
    if (partition + 1 >= vectorPartitions.size ())
    {
        HeapPartition* _partition = new HeapPartition (partition + 1);

        vectorPartitions.push_back (_partition);
    }

    return LULL;
}

MemoryBlock *HeapAllocator::allocate (unsigned long size)
{
    MemoryBlock *memBlock;
    
    memBlock = allocateInPartition (0, size);
    if (!memBlock)
    {
        /* Start Garbage Collection */
        ptrVM->doGarbageCollection ();
        
        /* Again allocate in the memory and this time
         * it shouldn't fail */
        memBlock = allocateInPartition (0, size);
    }

    return memBlock;
}

HeapPartition* HeapAllocator::getPartitionWithAddress (unsigned long address)
{
    /* Iterate over partitions */
    /* TODO: Improve this, Idea create a container which uses binary search
     * to insert an element and use binary search to find an element also
     * map wouldnt do the work. */

    for (int i = 0; i < vectorPartitions.size (); i++)
    {
        if (vectorPartitions[i]->getStartAddress () <= address &&
            vectorPartitions[i]->getStartAddress () + PARTITION_SIZE >= address)
        {
            return vectorPartitions[i];
        }
    }
    
    return LULL;
}

void HeapAllocator::freeAddress (unsigned long address)
{
    HeapPartition* partition;
    
    partition = getPartitionWithAddress (address);
    partition->freeMemory (address);
}

vector<AllocatedObject*>* HeapAllocator::reachableObjectsForPartition (int partitionNum)
{
    return vectorPartitions[partitionNum]->getReachableObjects ();
}