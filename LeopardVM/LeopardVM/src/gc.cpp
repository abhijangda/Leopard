#include "gc.h"

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
    vectorPartitions[0]->push_back (object);
    isPartitionFull (0);
}

void GarbageCollector::collect (int partitionNum)
{
    
}

void GarbageCollector::isPartitionFull (int partitionNum)
{
    if (partitionNum == vectorPartitions.size ())
    {
        return;
    }
    
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
    
    isPartitionFull (partitionNum + 1);
}