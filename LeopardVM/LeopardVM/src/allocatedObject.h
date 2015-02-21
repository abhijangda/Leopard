#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <list>

#include "classinfo.h"

#ifndef __Allocated_Object_H__
#define __Allocated_Object_H__

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

#endif /* __Allocated_Object_H__ */