#include "vm.h"

/* Maintain a list of free blocks and a list of all blocks 
 * sorted on the basis of start and ending address
 * Allocate from free blocks when required.
 * Use Best Fit to determine the free block to be assigned.
 */
HeapAllocator::HeapAllocator ()
{
    iterCurrBlock = blockLists.begin ();
}

MemoryBlock *HeapAllocator::createNewBlock (unsigned long size)
{
    MemoryBlock *memBlock = new MemoryBlock (size);
    
    /* TODO: Insert that block into the list in the sorted order
     * I think I should use something like a B+ Tree or a 
     * Red Black Tree. But for the time being let it be list
     */
    list<MemoryBlock*>::iterator iter;
    
    for (iter = blockLists.begin (); iter != blockLists.end (); iter++)
    {
        if ((*iter)->getStartPos () > memBlock->getStartPos ())
        {
            iter--;
            break;
        }
    }

    blockLists.insert (iter, memBlock);
    
    return memBlock;
}

MemoryBlock *HeapAllocator::allocate (unsigned long size)
{
    if (freeBlockLists.size () == 0)
    {
        /* No free blocks */

        return createNewBlock (size);
    }

    /* If there are free blocks */
    return LULL;
}