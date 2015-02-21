#include "main.h"
#include "allocatedObject.h"

#include <vector>

#ifndef __GC_H__
#define __GC_H__

typedef vector<AllocatedObject*> Partition;

class GarbageCollector
{
    public:
        void collect (int partitionNum);
        GarbageCollector ();
        ~GarbageCollector ();
        void pushObject (AllocatedObject* object);

    private:
        vector <Partition*> vectorPartitions;
        unsigned long sizeOfPartition;
        void isPartitionFull (int partitionNum);
};

#endif /* __GC_H__ */